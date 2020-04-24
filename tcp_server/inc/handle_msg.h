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
#define VERSION_STR_LENGTH 10
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
	static HandleMsg* GetInstance() {
		static HandleMsg instance;
		return &instance;
	}
	~HandleMsg();

	void Recieve_Preprocess(char *buffer, int len, int conn);

	void Recieve_login(char *buffer, int len, int conn);
	void Recieve_logout(char *buffer, int len, int conn);
	void Recieve_CallBackMessage(char *buffer, int len, int conn);
	void Recieve_ReissueMessage(char *buffer, int len, int conn);
	void Recieve_RealTimeMessage(char *buffer, int len, int conn);
	void Recieve_Platlogin(char *buffer, int len, int conn);
	void Recieve_HeartbeatMessage(char *buffer, int len, int conn);
	void Recieve_requireRSA(char *buffer, int len, int conn);
	void Recieve_AESKey(char *buffer, int len, int conn);

	void log_message(char *buffer,int len);

	void Send_responseCarLogin(int conn);
	void Send_responseRSA(int conn);
	void Send_responseAES(int conn);
	void Send_responsePlatlogin(int conn);

	// car cmd
	void SendCmd_lock(uint8_t *buffer, int &len);
	void SendCmd_find(uint8_t *buffer, int &len);
	void HandleSend(char *buffer,int *len);
	void SendCmd_airCondition(uint8_t *buffer, int &len);
	void SendCmd_lightControl(uint8_t *buffer, int &len);
	int Continue_in();

private:
	HandleMsg();
	std::string m_VERSION;
	std::string m_VIN;
	std::string m_TOKEN;
	uint8_t m_msgType;
	uint8_t m_encryptType;
};


#endif /* HANDLE_MSG_H_ */
