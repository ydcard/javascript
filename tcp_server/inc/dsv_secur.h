/*
 * dsv_secur.h
 *
 *  Created on: Aug 2, 2018
 *      Author: desay-sv
 */

#ifndef DSV_SECUR_H_
#define DSV_SECUR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/ossl_typ.h>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>
#include <string.h>
#include <string>
#include <iostream>
#include <cassert>



class HandleSafe{
public:
	static HandleSafe* GetInstance();
	~HandleSafe();
	void aes_encrypt(unsigned char* in,unsigned char* out,int length);
	void aes_decrypt(unsigned char* in,unsigned char* out, int length);
//	void handle_encrypt(char *in, int len, char *out);
//	void handle_decrypt(char *in, int len, char *out);
	int RSA_encrypt(char *in, char *out, int len);
	int RSA_decrypt(char *in, char *out);
	int Base64_encrypt(char *in, int len, char *out);//返回加密后的长度
	int Base64_decrypt(char *in, int len, char *out);//返回解密后的长度
	uint8_t CheckSum(uint8_t *buf, uint32_t len);
//	std::string m_RSAStr;
	std::string m_AESKEY;
private:
	HandleSafe();
	static HandleSafe* m_instance;
};



#endif /* DSV_SECUR_H_ */
