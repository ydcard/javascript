/*
 * handle_msg.h
 *
 *  Created on: Jul 30, 2018
 *      Author: desay-sv
 */

#ifndef HANDLE_MSG_H_
#define HANDLE_MSG_H_

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <thread>
#include <vector>
#include "dsv_log.h"
#include <string>
#include "ByteBuffer.h"
#include "dsv_secur.h"

#define BUFFER_SIZE 2018
#define STATIC_LENGTH 103
#define VIN_LENGTH 17
#define TOKEN_LENGTH 36
#define RSA_LENGTH 392

typedef enum msgType{
	RECIEVE_LOGIN 		= 0,
	RECIEVE_LOGOUT 		= 1,
	RECIEVE_HEARTBEAT 	= 2,
	RECIEVE_REALTIME 	= 3,
	RECIEVE_CALLBACK 	= 4,
	RECIEVE_REISSUE		= 5,
	RECIEVE_GETRSA 		= 6,
	RECIEVE_GETAES 		= 7,
	SENDCMD_LOCK 		= 10,
	SENDCMD_FIND 		= 11,
	SENDCMD_AIR 		= 12,
	SENDCMD_LIGHT		= 13,
	SEND_RSA			= 20,
	SEND_AES			= 21,
	SEND_LOGIN			= 22
}msgType_t;


class HandleMsg
{
public:
//	static HandleMsg* GetInstance();
	~HandleMsg();
	HandleMsg();
	void Recieve_Preprocess(char *buffer, int conn, int len);
	void Handle_login(char *buffer,int conn);
	void Handle_logout(char *buffer);
	void Recieve_CallBackMessage(char *buffer);
	void Recieve_ReissueMessage(char *buffer);
	void Recieve_RealTimeMessage(char *buffer, int len);
	void log_message(char *buffer,int len);
	void Recieve_Platlogin(char *buffer, int conn);
	void Recieve_HeartbeatMessage(char *buffer);
	void Recieve_requireRSA(char *buffer, int conn);
	void Recieve_AESKey(char *buffer,int len);
	void Send_requestRSA(int conn);
//	void Send_requestAES(int conn);
	void Send_requestPlatlogin(int conn);
	void SendCmd_lock(uint8_t *buffer, int &len);
	void SendCmd_find(uint8_t *buffer, int &len);
	void HandleSend(char *buffer,int *len);
	void SendCmd_airCondition(uint8_t *buffer, int &len);
	void SendCmd_lightControl(uint8_t *buffer, int &len);
	int Continue_in();

private:
	uint8_t m_msgType;
	uint8_t m_encryptType;
};

class ConfigServer
{
public:
	std::string m_VIN;
	std::string m_TOKEN;
//	std::string m_AESKEY;
	static ConfigServer* GetInstance();
	~ConfigServer();

private:
	static ConfigServer* m_configserver;
	ConfigServer();

};




#endif /* HANDLE_MSG_H_ */
