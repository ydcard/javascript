/*
 * dsv_secur.cpp
 *
 *  Created on: Aug 2, 2018
 *      Author: desay-sv
 */

#include "dsv_secur.h"

#define RSA_PUBKEY "yd_pub.pem"
#define RSA_PRIKEY "yd_pri.pem"

HandleSafe*HandleSafe::m_instance = NULL;

HandleSafe* HandleSafe::GetInstance(){
	if(m_instance == NULL)
	{
		m_instance = new HandleSafe();
	}
	return m_instance;
}

HandleSafe::HandleSafe(){
	m_AESKEY = "";
}

HandleSafe::~HandleSafe(){

}

uint8_t HandleSafe::CheckSum(uint8_t *buf, uint32_t len)
{
	uint32_t i;
	uint8_t checksum = 0;
	for (i = 0; i < len; i++)
	{
		checksum ^= *buf++;
	}
	return checksum;
}


void HandleSafe::aes_encrypt(unsigned char* in,unsigned char* out,int length){
//	int length = ((strlen((char *)in)+AES_BLOCK_SIZE-1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	AES_KEY aes;
	if(AES_set_encrypt_key((unsigned char*)(this->m_AESKEY.c_str()), AES_BLOCK_SIZE*8, &aes) < 0)
	{
		return;
	};
	int len = 0;
	/*循环解密*/
	while(len < length) {
		AES_encrypt(in+len, out+len, &aes);
		len += AES_BLOCK_SIZE;
	}
}

//采用AES_CBC加密
//void HandleSafe::aes_encrypt(unsigned char* in,unsigned char* out,int len){
//	AES_KEY aes;
//	if(AES_set_encrypt_key((unsigned char*)PG_AesKey, AES_BLOCK_SIZE*8, &aes) < 0)
//	{
//		return;
//	};
//	int len = strlen((char *)in);
//	AES_cbc_encrypt(in, out, len, &aes, (unsigned char *)PG_AesIv, AES_ENCRYPT);
//}

void HandleSafe::aes_decrypt(unsigned char* in,unsigned char* out, int length){
//	int length = ((Plen+AES_BLOCK_SIZE-1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	AES_KEY aes;
	if(AES_set_decrypt_key((unsigned char*)(this->m_AESKEY.c_str()), AES_BLOCK_SIZE*8, &aes) < 0)
	{
		return;
	};
	int len = 0;
	/*循环解密*/
	while(len < length) {
		AES_decrypt(in+len, out+len, &aes);
		len += AES_BLOCK_SIZE;
	}
}

//采用AES_CBC解密
//void HandleSafe::aes_decrypt(unsigned char* in,unsigned char* out ,int length){
//	AES_KEY aes;
//	if(AES_set_decrypt_key((unsigned char*)PG_AesKey, AES_BLOCK_SIZE*8, &aes) < 0)
//	{
//		return;
//	};
//	int len = strlen((char *)in);
//	AES_cbc_encrypt(in, out, len, &aes, (unsigned char *)PG_AesIv, AES_DECRYPT);
//}

//Base64加密方式一：
int HandleSafe::Base64_encrypt(char *in, int len, char *out){
	return EVP_EncodeBlock((unsigned char*)out, (const unsigned char*)in, len);
}

//Base64加密方式二：
//int HandleSafe::Base64_encrypt(char *in, int len, char *out){
//	BIO * bmem = NULL;
//	BIO * b64 = NULL;
//	BUF_MEM * bptr = NULL;
//
//	b64 = BIO_new(BIO_f_base64());
//
//	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);//0816：默认不换行
//
//	bmem = BIO_new(BIO_s_mem());
//	b64 = BIO_push(b64, bmem);
//	BIO_write(b64, in, len);
//	BIO_flush(b64);
//	BIO_get_mem_ptr(b64, &bptr);
//
////	char * buff = (char *)malloc(bptr->length + 1);
//	memcpy(out, bptr->data, bptr->length);
//	int a = bptr->length;
//
//	out[bptr->length] = 0;
//
//	BIO_free_all(b64);
//
//	return a;
//}

//Base64解密方式一：
//int HandleSafe::Base64_decrypt(char *in, int len, char *out){
//	return EVP_DecodeBlock((unsigned char*)out, (const unsigned char*)in, len);
//}

//Base64解密方式二：
int HandleSafe::Base64_decrypt(char *in, int len, char *out){
	BIO * b64 = NULL;
	BIO * bmem = NULL;
	char * buffer = (char *)malloc(len);
	memset(buffer, 0, len);

	b64 = BIO_new(BIO_f_base64());

	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);//0816：默认不换行

	bmem = BIO_new_mem_buf(in, len);
	bmem = BIO_push(b64, bmem);
	int a = BIO_read(bmem, buffer, len);

	BIO_free_all(bmem);
	memcpy(out,buffer,a);
	free(buffer);

	return a;
}


int HandleSafe::RSA_encrypt(char *in, char *out, int len){
    FILE* hPubKeyFile = fopen(RSA_PUBKEY,"r");
    if( hPubKeyFile == NULL )
    {
        return 0;
    }
    std::string strRet;
    RSA* pRSAPublicKey = RSA_new();
    if(PEM_read_RSA_PUBKEY(hPubKeyFile, &pRSAPublicKey, 0, 0) == NULL)
    {
        return 0;
    }

    int nLen = RSA_size(pRSAPublicKey);
    char* pEncode = new char[nLen + 1];
    int ret = RSA_public_encrypt(len, (const unsigned char*)in, (unsigned char*)pEncode, pRSAPublicKey, RSA_PKCS1_PADDING);
    if (ret >= 0)
    {
    	memcpy(out,pEncode,ret);
    }
    delete[] pEncode;
    RSA_free(pRSAPublicKey);
    fclose(hPubKeyFile);
    CRYPTO_cleanup_all_ex_data();//若在new之后使用free会造成内存泄露，为了避免这一情况的发生可以使用这个函数
    return ret;
}

int HandleSafe::RSA_decrypt(char *in, char *out){
    FILE* hPriKeyFile = fopen(RSA_PRIKEY,"r");
    if( hPriKeyFile == NULL )
    {
        return 0;
    }
    std::string strRet;
    RSA* pRSAPriKey = RSA_new();
    if(PEM_read_RSAPrivateKey(hPriKeyFile, &pRSAPriKey, 0, 0) == NULL)
    {
        return 0;
    }
    int nLen = RSA_size(pRSAPriKey);
    char* pDecode = new char[nLen+1];
    int ret = RSA_private_decrypt(nLen, (const unsigned char*)in, (unsigned char*)pDecode, pRSAPriKey, RSA_PKCS1_PADDING);
    if(ret >= 0)
    {
    	memcpy(out,pDecode,ret);
    }
    delete [] pDecode;
    RSA_free(pRSAPriKey);
    fclose(hPriKeyFile);
    CRYPTO_cleanup_all_ex_data();
    return ret;
}


