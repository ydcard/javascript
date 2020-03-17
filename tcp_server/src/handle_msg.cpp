/*
 * handle_msg.cpp
 *
 *  Created on: Jul 30, 2018
 *      Author: desay-sv
 */

#include "handle_msg.h"
#include <iostream>
#include <sstream>
#include <iomanip>
//#include "dsv_log.h"
#define REC "REC"

using namespace std;

std::ofstream ydfs;

#define YDLog(num,name,tag,variable)\
{\
	if(tag == 'd'){\
		char yd_buffer[15];\
    	sprintf(yd_buffer,"：%d\n",variable);\
    	if(ydfs.is_open() == false){\
    		ydfs.open("ydlog.txt",fstream::app);\
		}\
		ydfs << std::left << setw(25+num) << name << yd_buffer;\
	}\
    else if(tag == 'x'){\
		char yd_buffer[15];\
    	sprintf(yd_buffer,"：%#X\n",variable);\
    	if(ydfs.is_open() == false){\
    		ydfs.open("ydlog.txt",fstream::app);\
		}\
		ydfs << std::left << setw(25+num) << name << yd_buffer;\
    }\
}\


YLOG_SET(false,true,VL_ALL)

ConfigServer *ConfigServer::m_configserver = NULL;

ConfigServer* ConfigServer::GetInstance()
{
	if (NULL == m_configserver)
	{
		m_configserver = new ConfigServer();
	}

	return m_configserver;
}

ConfigServer::ConfigServer(){
	this->m_VIN = "AAAAAAAAAA0009990";
	this->m_TOKEN = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
}

HandleMsg::HandleMsg(){
	this->m_msgType = 0;
	this->m_encryptType = 0;
}

HandleMsg::~HandleMsg(){

}

void HandleMsg::Handle_login(char *buffer, int conn){
	YLOG(VL_INFO,REC,"Recieve Car login\n");
	ByteBuffer *bccpkt = new ByteBuffer();
	bccpkt->put((uint8_t) 0x04);
	bccpkt->put((uint8_t) 0x01);
	bccpkt->put((uint8_t) 0x01);
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_VIN.c_str(),VIN_LENGTH); //VIN:VIN, 应符合 GB16735 的规定 AAAAAAAAAA0009990

	bccpkt->put((uint8_t) 0x01);

	//Pangoo:添加TOKEN头
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_TOKEN.c_str(),TOKEN_LENGTH);

	bccpkt->putShort(htons(6)); //YinDi:数据长度

	time_t now;  //实例化time_t结构
	time(&now);  //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
//	now = now + 8 * 3600;  //GMT+8*3600，等于北京时间
	struct tm *timenow;  //实例化tm结构指针
	struct tm timenowInstance;
	timenow = gmtime_r(&now,&timenowInstance);

	bccpkt->put(timenow->tm_year + 1900 - 2000); //年 0～99
	bccpkt->put(timenow->tm_mon + 1); //月 1～12
	bccpkt->put(timenow->tm_mday); //日 1～31
	bccpkt->put(timenow->tm_hour); //时 0～23
	bccpkt->put(timenow->tm_min); //分 0～59
	bccpkt->put(timenow->tm_sec); //秒 0～59

	uint8_t *BCCBuffer = (uint8_t*) malloc(sizeof(uint8_t) * bccpkt->size());
	bccpkt->getBytes(BCCBuffer, bccpkt->size());
	uint8_t checksum = 0;
	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, bccpkt->size());

	ByteBuffer *wholepkt = new ByteBuffer();
	wholepkt->put((uint8_t) 0x23);  //起始符:固定为ASCII字符‘#’,用“0x23”表示
	wholepkt->put(bccpkt);
	wholepkt->put(checksum);

	uint8_t *senBuffer = (uint8_t*) malloc(sizeof(uint8_t) * wholepkt->size());
	wholepkt->getBytes(senBuffer, wholepkt->size());

	int flag = send(conn, senBuffer, wholepkt->size(), 0);

	free(senBuffer);
	free(BCCBuffer);
	wholepkt->clear();
	bccpkt->clear();
	delete wholepkt;
	delete bccpkt;
}

void HandleMsg::Handle_logout(char *buffer){
	YLOG(VL_INFO,REC,"logout message is %s\n",buffer);
}

void HandleMsg::Recieve_CallBackMessage(char *buffer){
	YLOG(VL_INFO,REC,"Recievec CallBack message\n");
}

void HandleMsg::Recieve_RealTimeMessage(char *buffer, int len){
	YLOG(VL_INFO,REC,"Recieve RealTimeMessage message\n");
	ByteBuffer *wholeBuffer = new ByteBuffer();
	wholeBuffer->putBytes((unsigned char*)buffer,len);
	int index = wholeBuffer->find(ntohs((short) 0x2304));//下表开始的位置
	uint8_t encryptionWay = wholeBuffer->get(index + 21);//获取加密方式
	short dataLength = ntohs(wholeBuffer->getShort(index + 58));//获取数据单元长度
//	printf("数据单元长度 is %d\n",dataLength);
	uint8_t checkbit = wholeBuffer->get(index + 60 + dataLength);//获取到的校验和
//	验证校验和
	uint8_t *BCCBuffer = (uint8_t *) malloc(sizeof(uint8_t) * (60 + dataLength - 1));
	wholeBuffer->getBytes(BCCBuffer, 60 + dataLength - 1, index + 1);
	uint8_t checksum = 0;
	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, 60 + dataLength - 1);
	free(BCCBuffer);
	if(checksum == checkbit){
//	进行RSA解码
//		int length = ((dataLength+AES_BLOCK_SIZE-1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
		char *InBuffer = (char*)malloc(dataLength);
		char *OutBuffer = (char*)malloc(dataLength);
		wholeBuffer->getBytes((unsigned char*)InBuffer, dataLength, index + 60);//获取TBOX传达的数据
		HandleSafe::GetInstance()->aes_decrypt((unsigned char*)InBuffer,(unsigned char*)OutBuffer,dataLength);
		this->log_message(OutBuffer,dataLength);
		free(InBuffer);
		free(OutBuffer);
	}
	wholeBuffer->clear();
	delete wholeBuffer;
}

void HandleMsg::log_message(char *buff,int len)
{
	uint8_t *buffer = (uint8_t *)buff;
	buffer = buffer + 6;
	ydfs.open("ydlog.txt",std::ios::app);
	ydfs << "~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	timeval __tv;
	gettimeofday(&__tv, NULL);
	__tv.tv_sec = __tv.tv_sec;
	char tmpbuf[128];
	strftime(tmpbuf,128,"[%F %T] ",localtime(&__tv.tv_sec));
	ydfs << tmpbuf << endl;
	++buffer;
    YDLog(4,"车辆状态",'x',*buffer++);
    YDLog(4,"充电状态",'x',*buffer++);
    YDLog(4,"运行模式",'x',*buffer++);
    uint16_t __speed = (*buffer++ << 8) + (*buffer++);
    YDLog(2,"车速",'d',__speed);
    uint32_t __mile = (*buffer++ << 24) + (*buffer++ << 16) + (*buffer++ << 8) + (*buffer++);
	YDLog(4,"累计里程",'d',__mile);//累计里程
	buffer = buffer + 6;
	YDLog(2,"档位",'x',*buffer++);//档位
	buffer = buffer + 2;
	YDLog(6,"加速踏板行程",'x',*buffer++);//加速踏板行程
	YDLog(6,"制动踏板行程",'x',*buffer++);//制动踏板行程
	YDLog(3,"转向灯",'x',*buffer++);//转向灯
	uint16_t __Vacc = (*buffer++ << 8) + (*buffer++);
	YDLog(5,"纵向加速度",'d',__Vacc);//纵向加速度
	uint16_t __Racc = (*buffer++ << 8) + (*buffer++);
	YDLog(5,"侧向加速度",'d',__Racc);//侧向加速度
	uint16_t __IMU = (*buffer++ << 8) + (*buffer++);
	YDLog(5,"IMU横摆角速度",'d',__IMU);//IMU横摆角速度
	uint16_t __WheelAngle = (*buffer++ << 8) + (*buffer++);
	YDLog(5,"方向盘转角",'d',__WheelAngle);//方向盘转角
	uint16_t __WheelSpeed = (*buffer++ << 8) + (*buffer++);
	YDLog(5,"方向盘转速",'d',__WheelSpeed);//方向盘转速
	uint16_t __RealAcc = (*buffer++ << 8) + (*buffer++);
	YDLog(5,"实际加速度",'d',__RealAcc);//实际加速度
	YDLog(7,"实际加速度有效",'x',*buffer++);//实际加速度有效
	uint16_t __Yaw = (*buffer++ << 8) + (*buffer++);
	YDLog(7,"车身横摆角速度",'d',__Yaw);//车身横摆角速度
	YDLog(9,"防抱死制动系统激活",'x',*buffer++);//防抱死制动系统激活
	YDLog(9,"防抱死制动系统失败",'x',*buffer++);//防抱死制动系统失败
	YDLog(11,"巡航和限速系统开关状态",'x',*buffer++);//巡航和限速系统开关状态
	YDLog(6,"制动状态信号",'x',*buffer++);//制动状态信号
	YDLog(4,"钥匙档位",'x',*buffer++);//钥匙档位
	YDLog(6,"轮胎低压指示",'x',*buffer++);//轮胎低压指示
	YDLog(4,"怠速状态",'x',*buffer++);//怠速状态
	YDLog(7,"乘客安全带状态",'x',*buffer++);//乘客安全带状态
	YDLog(8,"驾驶员安全带状态",'x',*buffer++);//驾驶员安全带状态
	buffer = buffer + 1;

	//发动机
	YDLog(5,"发动机状态",'x',*buffer++);//发动机状态
	uint16_t __QuZhou = (*buffer++ << 8) + (*buffer++);
	YDLog(4,"曲轴转速",'d',__QuZhou);//曲轴转速
	uint16_t __Oil = (*buffer++ << 8) + (*buffer++);
	YDLog(4,"瞬时油耗",'d',__Oil);//瞬时油耗
	buffer = buffer + 1;

	//车辆位置
	YDLog(4,"定位状态",'x',*buffer++);//定位状态
	uint32_t __long = (*buffer++ << 24) + (*buffer++ << 16) + (*buffer++ << 8) + (*buffer++);
	YDLog(2,"经度",'d',__long);//经度
	uint32_t __lat = (*buffer++ << 24) + (*buffer++ << 16) + (*buffer++ << 8) + (*buffer++);
	YDLog(2,"纬度",'d',__lat);//纬度
	buffer = buffer + 1;

	//报警数据
	YDLog(4,"胎压预警",'x',*buffer++);//胎压预警
	YDLog(9,"发动机后置氧传感器",'x',*buffer++);//发动机后置氧传感器需要诊断援助
	YDLog(4,"刹车过热",'x',*buffer++);//刹车过热
	YDLog(5,"变速箱预警",'x',*buffer++);//变速箱预警
	YDLog(5,"离合器预警",'x',*buffer++);//离合器预警
	YDLog(3,"俯仰角",'x',*buffer++);//俯仰角
	YDLog(3,"侧倾角",'x',*buffer++);//侧倾角
	buffer = buffer + 1;

	//81 Pangoo自定义数据
	YDLog(4,"门锁状态",'x',*buffer++);
	YDLog(4,"车窗状态",'x',*buffer++);
	uint32_t __light = (*buffer++ << 24) + (*buffer++ << 16) + (*buffer++ << 8) + (*buffer++);
	YDLog(4,"车灯状态",'d',__light);
	YDLog(5,"发动机状态",'x',*buffer++);
	YDLog(2,"水温",'x',*buffer++);
	YDLog(2,"空调",'x',*buffer++);
	buffer = buffer + 1;
	uint16_t __HoldMile = (*buffer++ << 8) + (*buffer++);
	YDLog(4,"续航里程",'d',__HoldMile);
	YDLog(4,"手刹状态",'x',*buffer++);
	YDLog(4,"雨刮状态",'x',*buffer++);
	ydfs.close();
}

void HandleMsg::Recieve_HeartbeatMessage(char *buffer){
	YLOG(VL_INFO,REC,"Recieve Heartbeat message\n");
}


void HandleMsg::SendCmd_lock(uint8_t *buffer, int &len){
	int d0;
	len++;
	buffer[len] = 0x01;
	len++;
	printf("Please enter lock command(0：无动作；1：上锁；2：解锁；3：保留) :");
	scanf("%d",&d0);
	buffer[len] = d0;
	fgetc(stdin);
	len++;
}

void HandleMsg::SendCmd_find(uint8_t *buffer, int &len){
	int d0;
	len++;
	buffer[len] = 0x01;
	len++;
	printf("Please enter find command(0：无动作；1：寻车；2：停止寻车；3：保留) :");
	scanf("%d",&d0);
	buffer[len] = d0;
	fgetc(stdin);
	len++;
}

void HandleMsg::SendCmd_airCondition(uint8_t *buffer, int &len){
	int d0;
	len++;
	buffer[len] = 0x01;
	len++;
	uint8_t c1,c2,c3;
	printf("输入空调风速：(0~7）-->");
	scanf("%d",&d0);
	c1 = d0;
	fgetc(stdin);
	printf("输入空调温度：(0~7）-->");
	scanf("%d",&d0);
	c2 = d0;
	fgetc(stdin);
	printf("输入空调模式：(0~3）-->");
	scanf("%d",&d0);
	c3 = d0;
	fgetc(stdin);
	uint8_t c4 = (c1<<5) | (c2<<2) | c3;
	buffer[len] = c4;
	len++;
}

void HandleMsg::SendCmd_lightControl(uint8_t *buffer, int &len){
	int d0;
	len++;
	buffer[len] = 0x04;
	len++;
	printf("Please enter red value(0~255) :");
	scanf("%d",&d0);
	buffer[len] = d0;
	fgetc(stdin);
	len++;
	printf("Please enter green value(0~255) :");
	scanf("%d",&d0);
	buffer[len] = d0;
	fgetc(stdin);
	len++;
	printf("Please enter blue value(0~255) :");
	scanf("%d",&d0);
	buffer[len] = d0;
	fgetc(stdin);
	len++;
	buffer[len] = 0;
	len++;
}

void HandleMsg::Recieve_requireRSA(char *buffer, int conn){
//	printf("收到RSA公钥获取请求\n");
	YLOG(VL_INFO,"Recieve require RSA","Recieve require RSA message\n");
	Send_requestRSA(conn);
}

void HandleMsg::Recieve_Platlogin(char *buffer, int conn){
//	printf("接收到了平台登录请求\n");
	YLOG(VL_INFO,REC,"Recieve platform login message\n");
	Send_requestPlatlogin(conn);
}

void HandleMsg::Recieve_AESKey(char *buffer,int len){
//	printf("收到AES密钥\n");
	YLOG(VL_INFO,REC,"Recieve AES message\n");
//	printf("AES key len is %d\n",len);
//	先获取到接受到的buffer数据处理：获取加密方式，获取数据长度，获取校验和
	ByteBuffer *wholeBuffer = new ByteBuffer();
	wholeBuffer->putBytes((unsigned char*)buffer,len);
	int index = wholeBuffer->find(ntohs((short) 0x2304));//下表开始的位置
	uint8_t encryptionWay = wholeBuffer->get(index + 21);//获取加密方式
	short dataLength = ntohs(wholeBuffer->getShort(index + 58));//获取数据单元长度
//	printf("数据单元长度 is %d\n",dataLength);
	uint8_t checkbit = wholeBuffer->get(index + 60 + dataLength);//获取到的校验和
//	验证校验和
	uint8_t *BCCBuffer = (uint8_t *) malloc(sizeof(uint8_t) * (60 + dataLength - 1));
	wholeBuffer->getBytes(BCCBuffer, 60 + dataLength - 1, index + 1);
	uint8_t checksum = 0;
	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, 60 + dataLength - 1);
	free(BCCBuffer);
	if(checksum == checkbit){
//	进行RSA解码
		uint8_t *AESBuffer = (uint8_t *) malloc(344);
		wholeBuffer->getBytes(AESBuffer, 344, index + 66);//获取TBOX传达的数据
		char *RSAStr = (char*)malloc(256);
		char *AESBuf = (char*)malloc(100);
		int De_Base64Len = HandleSafe::GetInstance()->Base64_decrypt((char*)AESBuffer,344,RSAStr);
//		printf("Base64 len is %d\n",De_Base64Len);
		int De_RSALen = HandleSafe::GetInstance()->RSA_decrypt((char*)RSAStr,AESBuf);
//		printf("RSA len is %d\n",De_RSALen);
//		std::string AES_Str;
//		AES_Str = std::string(AESBuf,strlen(AESBuf));
		HandleSafe::GetInstance()->m_AESKEY = std::string(AESBuf,De_RSALen);
		std::cout<<HandleSafe::GetInstance()->m_AESKEY<<std::endl;
		free(AESBuffer);
		free(RSAStr);
		free(AESBuf);
	}
	wholeBuffer->clear();
	delete wholeBuffer;
}

void HandleMsg::Recieve_ReissueMessage(char *buffer){
//	printf("收到了补发数据\n");
	YLOG(VL_INFO,REC,"Recieve Reissue message\n");
}

void HandleMsg::Send_requestRSA(int conn){
	YLOG(VL_INFO,"SEND","Send RSA KEY\n");
	ByteBuffer *bccpkt = new ByteBuffer();
	bccpkt->put((uint8_t) 0x04);
	bccpkt->put((uint8_t) 0xC5);
	bccpkt->put((uint8_t) 0x01);
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_VIN.c_str(),VIN_LENGTH); //VIN:VIN, 应符合 GB16735 的规定 AAAAAAAAAA0009990

	bccpkt->put((uint8_t) 0x01);

	//Pangoo:添加TOKEN头
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_TOKEN.c_str(),TOKEN_LENGTH);

//		bccpkt->putShort(htons(0x40)); //数据长度
	bccpkt->putShort(htons(RSA_LENGTH+6)); //YinDi:数据长度

	time_t now;  //实例化time_t结构
	time(&now);  //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
//	now = now + 8 * 3600;  //GMT+8*3600，等于北京时间
	struct tm *timenow;  //实例化tm结构指针
	struct tm timenowInstance;
	timenow = gmtime_r(&now,&timenowInstance);

	bccpkt->put(timenow->tm_year + 1900 - 2000); //年 0～99
	bccpkt->put(timenow->tm_mon + 1); //月 1～12
	bccpkt->put(timenow->tm_mday); //日 1～31
	bccpkt->put(timenow->tm_hour); //时 0～23
	bccpkt->put(timenow->tm_min); //分 0～59
	bccpkt->put(timenow->tm_sec); //秒 0～59

	char buffer[BUFFER_SIZE];
	std::string RSAStr;
	FILE *fp = fopen("yd_pub.pem","r");
	if(NULL == fp){
		return;
	}

	while(NULL != fgets(buffer,64,fp)){
		if(strncmp(buffer, "-----", 5) == 0)
		{
			continue;
		}
		else{
			for(int i=0;i<64;i++)
				if(buffer[i]=='\n')
					buffer[i]='\0';
			RSAStr = RSAStr + buffer;
			continue;
		}
	}
	bccpkt->putBytes((uint8_t*)RSAStr.c_str(),RSA_LENGTH);

	uint8_t *BCCBuffer = (uint8_t*) malloc(sizeof(uint8_t) * bccpkt->size());
	bccpkt->getBytes(BCCBuffer, bccpkt->size());
	uint8_t checksum = 0;
	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, bccpkt->size());

	ByteBuffer *wholepkt = new ByteBuffer();
	wholepkt->put((uint8_t) 0x23);  //起始符:固定为ASCII字符‘#’,用“0x23”表示
	wholepkt->put(bccpkt);
	wholepkt->put(checksum);

	uint8_t *senBuffer = (uint8_t*) malloc(sizeof(uint8_t) * wholepkt->size());
	wholepkt->getBytes(senBuffer, wholepkt->size());

	int flag = send(conn, senBuffer, wholepkt->size(), 0);

	free(senBuffer);
	free(BCCBuffer);
	wholepkt->clear();
	bccpkt->clear();
	delete wholepkt;
	delete bccpkt;
	senBuffer = NULL;
	BCCBuffer = NULL;
}

void HandleMsg::Send_requestPlatlogin(int conn){
	ByteBuffer *bccpkt = new ByteBuffer();
	bccpkt->put((uint8_t) 0x04);
	bccpkt->put((uint8_t) 0x05);
	bccpkt->put((uint8_t) 0x01);
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_VIN.c_str(),VIN_LENGTH); //VIN:VIN, 应符合 GB16735 的规定 AAAAAAAAAA0009990

	bccpkt->put((uint8_t) 0x01);

	//Pangoo:添加TOKEN头
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_TOKEN.c_str(),TOKEN_LENGTH);

//		bccpkt->putShort(htons(0x40)); //数据长度
	bccpkt->putShort(htons(6)); //YinDi:数据长度

	time_t now;  //实例化time_t结构
	time(&now);  //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
//	now = now + 8 * 3600;  //GMT+8*3600，等于北京时间
	struct tm *timenow;  //实例化tm结构指针
	struct tm timenowInstance;
	timenow = gmtime_r(&now,&timenowInstance);

	bccpkt->put(timenow->tm_year + 1900 - 2000); //年 0～99
	bccpkt->put(timenow->tm_mon + 1); //月 1～12
	bccpkt->put(timenow->tm_mday); //日 1～31
	bccpkt->put(timenow->tm_hour); //时 0～23
	bccpkt->put(timenow->tm_min); //分 0～59
	bccpkt->put(timenow->tm_sec); //秒 0～59

	uint8_t *BCCBuffer = (uint8_t*) malloc(sizeof(uint8_t) * bccpkt->size());
	bccpkt->getBytes(BCCBuffer, bccpkt->size());
	uint8_t checksum = 0;
	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, bccpkt->size());

	ByteBuffer *wholepkt = new ByteBuffer();
	wholepkt->put((uint8_t) 0x23);  //起始符:固定为ASCII字符‘#’,用“0x23”表示
	wholepkt->put(bccpkt);
	wholepkt->put(checksum);

	uint8_t *senBuffer = (uint8_t*) malloc(sizeof(uint8_t) * wholepkt->size());
	wholepkt->getBytes(senBuffer, wholepkt->size());

	int flag = send(conn, senBuffer, wholepkt->size(), 0);

	free(senBuffer);
	free(BCCBuffer);
	wholepkt->clear();
	bccpkt->clear();
	delete wholepkt;
	delete bccpkt;
	senBuffer = NULL;
	BCCBuffer = NULL;
}

//void HandleMsg::Send_requestAES(int conn){
//	ByteBuffer *bccpkt = new ByteBuffer();
//	bccpkt->put((uint8_t) 0x04);
//	bccpkt->put((uint8_t) 0xC6);
//	bccpkt->put((uint8_t) 0x01);
//	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_VIN.c_str(),VIN_LENGTH); //VIN:VIN, 应符合 GB16735 的规定 AAAAAAAAAA0009990
//
//	bccpkt->put((uint8_t) 0x00);
//
//	//Pangoo:添加TOKEN头
//	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_TOKEN.c_str(),TOKEN_LENGTH);
//
//	bccpkt->putShort(htons(6)); //YinDi:数据长度
//
//	time_t now;  //实例化time_t结构
//	time(&now);  //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
////	now = now + 8 * 3600;  //GMT+8*3600，等于北京时间
//	struct tm *timenow;  //实例化tm结构指针
//	struct tm timenowInstance;
//	timenow = gmtime_r(&now,&timenowInstance);
//
//	ByteBuffer *databuffer = new ByteBuffer();
//
//	databuffer->put(timenow->tm_year + 1900 - 2000); //年 0～99
//	databuffer->put(timenow->tm_mon + 1); //月 1～12
//	databuffer->put(timenow->tm_mday); //日 1～31
//	databuffer->put(timenow->tm_hour); //时 0～23
//	databuffer->put(timenow->tm_min); //分 0～59
//	databuffer->put(timenow->tm_sec); //秒 0～59
//	char *databuf = (char*)malloc(sizeof(char) * databuffer->size());
//	databuffer->getBytes((unsigned char*)databuf, databuffer->size());
//	int length = ((databuffer->size()+AES_BLOCK_SIZE-1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
//	char *outbuf = (char*)malloc(sizeof(char) * length);
//
//	HandleSafe::GetInstance()->aes_encrypt(databuf,(char*)ConfigServer::GetInstance()->m_AESKEY.c_str(),outbuf);
//	bccpkt->putBytes((uint8_t*)outbuf, length);
//
//	uint8_t *BCCBuffer = (uint8_t*) malloc(sizeof(uint8_t) * bccpkt->size());
//	bccpkt->getBytes(BCCBuffer, bccpkt->size());
//	uint8_t checksum = 0;
//	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, bccpkt->size());
//
//	ByteBuffer *wholepkt = new ByteBuffer();
//	wholepkt->put((uint8_t) 0x23);  //起始符:固定为ASCII字符‘#’,用“0x23”表示
//	wholepkt->put(bccpkt);
//	wholepkt->put(checksum);
//
//	uint8_t *senBuffer = (uint8_t*) malloc(sizeof(uint8_t) * wholepkt->size());
//	wholepkt->getBytes(senBuffer, wholepkt->size());
//
//	int flag = send(conn, senBuffer, wholepkt->size(), 0);
//
//	free(senBuffer);
//	free(outbuf);
//	free(databuf);
//	free(BCCBuffer);
//	delete databuffer;
//	delete wholepkt;
//	delete bccpkt;
//	databuffer = NULL;
//	outbuf = NULL;
//	databuf = NULL;
//	senBuffer = NULL;
//	BCCBuffer = NULL;
//	wholepkt = NULL;
//	bccpkt = NULL;
//}

int HandleMsg::Continue_in(){
	int yflag = 0;
	char c0;
	while(1){
		printf("do you wanna continue ?(y/n)");
		scanf("%c",&c0);
		if(c0 == 'n'){
			yflag = 0;
			printf("message is send !!!!!\n");
			break;
		}
		else if(c0 == 'y'){
			yflag = 1;
			break;
		}
		else{
			fgetc(stdin);
			printf("please enter y/n!!!\n");
		}
	}
	fgetc(stdin);
	return yflag;
}


void HandleMsg::HandleSend(char *buffer ,int *len){
	uint8_t sum = 0;
//	int d0;
	int d3;
	int d1 = 1;
//	int d2 = 1;
	char c0;
	int yflag = 0;
	int dflag = 0;
	std::vector<int> CmdNum;
	ByteBuffer *bccpkt = new ByteBuffer();
	bccpkt->put((uint8_t) 0x04);
	bccpkt->put((uint8_t) 0xC3);
	bccpkt->put((uint8_t) 0xFE);
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_VIN.c_str(),VIN_LENGTH);
	bccpkt->put((uint8_t) 0x03);
	bccpkt->putBytes((uint8_t*)ConfigServer::GetInstance()->m_TOKEN.c_str(),TOKEN_LENGTH);

	ByteBuffer *UnitDatapkt = new ByteBuffer();
	time_t now;  //实例化time_t结构
	time(&now);  //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now
	now = now + 8 * 3600;  //GMT+8*3600，等于北京时间
	struct tm *timenow;  //实例化tm结构指针
	struct tm timenowInstance;
	timenow = gmtime_r(&now,&timenowInstance);
	UnitDatapkt->put(timenow->tm_year + 1900 - 2000); //年 0～99
	UnitDatapkt->put(timenow->tm_mon + 1); //月 1～12
	UnitDatapkt->put(timenow->tm_mday); //日 1～31
	UnitDatapkt->put(timenow->tm_hour); //时 0～23
	UnitDatapkt->put(timenow->tm_min); //分 0～59
	UnitDatapkt->put(timenow->tm_sec); //秒 0～59
	uint8_t *_uploadbuf = (uint8_t*)malloc(36);
	memset(_uploadbuf,0,36);
	for(int i=0; i<4; i++){
		sum++;
		_uploadbuf[0] = sum;
		while(1){
			dflag = 0;
			printf("Please enter command number :( 1:上锁   2:寻车   3:空调   4:氛围灯 )");
			scanf("%d",&d3);
			for(int _i = 0;_i<CmdNum.size();_i++){
//				printf("CmdNum[%d] is %d\n",_i,CmdNum[_i]);
				if(CmdNum[_i] == d3){
					dflag = 1;
					printf("Re-enter the command number!!!\n");
					break;
				}
			}
			if(dflag == 0){
				break;
			}
		}
		CmdNum.push_back(d3);
//		printf("d3 is %d\n",d3);
//		printf("size is %ld\n",CmdNum.size());
		_uploadbuf[d1] = d3;
		fgetc(stdin);
		if(_uploadbuf[d1] == 1){
			SendCmd_lock(_uploadbuf,d1);
//			d1 = d2;
//			memcpy(buffer,_uploadbuf,sizeof(_uploadbuf));
			yflag = Continue_in();
			if(yflag == 0){
				break;
			}
		}
		else if(_uploadbuf[d1] == 2){
			SendCmd_find(_uploadbuf,d1);
//			memcpy(buffer,_uploadbuf,sizeof(_uploadbuf));
			yflag = Continue_in();
			if(yflag == 0){
				break;
			}
		}
		else if(_uploadbuf[d1] == 3){
			SendCmd_airCondition(_uploadbuf,d1);
//			memcpy(buffer,_uploadbuf,sizeof(_uploadbuf));
			yflag = Continue_in();
			if(yflag == 0){
				break;
			}
		}
		else if(_uploadbuf[d1] == 4){
			SendCmd_lightControl(_uploadbuf,d1);
//			memcpy(buffer,_uploadbuf,sizeof(_uploadbuf));
			yflag = Continue_in();
			if(yflag == 0){
				break;
			}
		}
	}
	UnitDatapkt->putBytes(_uploadbuf,d1-1);
	int __length = UnitDatapkt->size();
	int Buflength = ((__length+AES_BLOCK_SIZE-1)/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	uint8_t *en_UnitDatapkt = (uint8_t*)malloc(sizeof(uint8_t)*__length);
	uint8_t *UnitDatapkt_in = (uint8_t*)malloc(sizeof(uint8_t)*Buflength);
	uint8_t *UnitDatapkt_out = (uint8_t*)malloc(sizeof(uint8_t)*Buflength);
	UnitDatapkt->getBytes(en_UnitDatapkt,UnitDatapkt->size());
	memset(UnitDatapkt_in, 0, Buflength);
	memcpy(UnitDatapkt_in, en_UnitDatapkt, __length);
	HandleSafe::GetInstance()->aes_encrypt(UnitDatapkt_in,UnitDatapkt_out,Buflength);

	bccpkt->putShort(htons(Buflength)); //数据长度
	bccpkt->putBytes(UnitDatapkt_out,Buflength); //经过加密后的数据

	uint8_t *BCCBuffer = (uint8_t *) malloc(sizeof(uint8_t) * bccpkt->size());
	bccpkt->getBytes(BCCBuffer, bccpkt->size());

	uint8_t checksum = 0;
	checksum = HandleSafe::GetInstance()->CheckSum(BCCBuffer, bccpkt->size());

	//实时数据和补发数据上传
	ByteBuffer *wholepkt = new ByteBuffer();
	wholepkt->put((uint8_t) 0x23);
	wholepkt->put(bccpkt);
	wholepkt->put(checksum);

	uint8_t *sendBuffer = (uint8_t *)malloc(sizeof(uint8_t)*wholepkt->size());
	wholepkt->getBytes(sendBuffer,wholepkt->size());

	memcpy(buffer,sendBuffer,wholepkt->size());
	*len = wholepkt->size();

//	int flag = send(conn, sendBuffer, wholepkt->size(), 0);

	free(_uploadbuf);
	free(en_UnitDatapkt);
	free(UnitDatapkt_in);
	free(UnitDatapkt_out);
	free(BCCBuffer);
	free(sendBuffer);
	UnitDatapkt->clear();
	bccpkt->clear();
	wholepkt->clear();
	delete UnitDatapkt;
	delete bccpkt;
	delete wholepkt;
}

void HandleMsg::Recieve_Preprocess(char *buffer, int conn, int len){
//	this->m_encryptType = buffer[21];
//	this->m_msgType = buffer[2];
	switch((uint8_t)buffer[2]){
	case 0x01:
		this->Handle_login(buffer,conn);//应答车辆登陆成功
		break;
	case 0x02:
		this->Recieve_RealTimeMessage(buffer, len);//记录这部分上传的周期数据
		break;
	case 0x03:
		this->Recieve_ReissueMessage(buffer);
		break;
	case 0x04:
		this->Handle_logout(buffer);
		break;
	case 0x05:
		this->Recieve_Platlogin(buffer,conn);//返回TOKEN值
		break;
	case 0x07:
		this->Recieve_HeartbeatMessage(buffer);
		break;
	case 0xC3:
		this->Recieve_CallBackMessage(buffer);//记录这部分上传的车辆控制应答数据
		break;
	case 0xC5:
		this->Recieve_requireRSA(buffer, conn);//需要返回RSA公钥
		break;
	case 0xC6:
		this->Recieve_AESKey(buffer,len);//保留AES秘钥
		break;
	default:
		break;
	}
}



