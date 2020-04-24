/*
 * SocketManager.cpp
 *
 *  Created on: Mar 19, 2020
 *      Author: uidq0947
 */

#include "SocketManager.h"

void SocketManager::exitConnect()
{
	m_bShouldExit = true;
	close(m_iConnectId);
}

void SocketManager::SendCmdToTboxThread()
{
	std::cout << "send cmd thread" << std::endl;
	if (m_bSendCmdThreadRunning) {
		return;
	}
	m_bSendCmdThreadRunning = true;
	char *buffer = new char[BUFFER_SIZE];
	while (1) {
		memset(buffer,0,sizeof(buffer));
		int plen;
		char s[100];
		scanf("%s",s);
		if (strncmp(s,"0",1)!=0) {
			printf("please enter 0 to begin send message\n");
			continue;
		}
		HandleMsg::GetInstance()->HandleSend(buffer,&plen);
		int len = send(m_iConnectId, buffer, plen, 0);

		if(m_bShouldExit){
			break;
		}
	}
	delete buffer;
	buffer = NULL;
	m_bSendCmdThreadRunning = false;
}

void SocketManager::RevMsgFromTboxThread()
{
	std::cout << "recv msg thread" << std::endl;
	while (true) {
		char buffer[BUFFER_SIZE];
		memset(buffer,0,sizeof(buffer));
		int len = recv(m_iConnectId, buffer, sizeof(buffer),0);
		if(len > 0){
			HandleMsg::GetInstance()->Recieve_Preprocess(buffer,len,m_iConnectId);
		}
		else{
			std::cout << "error len is " << len << std::endl;
			exitConnect();
			break;
		}
	}
}

void SocketManager::init(int _connectId)
{
	std::cout << "init" << std::endl;
	m_iConnectId = _connectId;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::thread StartSendCmdToTbox(&SocketManager::SendCmdToTboxThread, this);
	StartSendCmdToTbox.detach();
	std::thread StartRecFromTbox(&SocketManager::RevMsgFromTboxThread, this);
	StartRecFromTbox.join();
}


