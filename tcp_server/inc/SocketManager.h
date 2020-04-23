/*
 * SocketManager.h
 *
 *  Created on: Mar 19, 2020
 *      Author: yindi
 * Description: 
 */

#ifndef SOCKETMANAGER_H_
#define SOCKETMANAGER_H_

#include <string>
#include <thread>
#include <iostream>
#include <unistd.h>
#include "handle_msg.h"

class SocketManager
{
public:
	SocketManager(){}
	~SocketManager(){}

	void init(int _connectId){
		m_iConnectId = _connectId;
		std::thread StartSendCmdToTbox(&SocketManager::SendCmdToTboxThread, this);
		StartSendCmdToTbox.detach();

		std::thread StartRecFromTbox(&SocketManager::RevMsgFromTboxThread, this);
	}

	void exitConnect(){
		m_bShouldExit = true;
		close(m_iConnectId);
	}

	void SendCmdToTboxThread(){
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

	void RevMsgFromTboxThread(){
		while (true) {
		    char buffer[BUFFER_SIZE];
		    memset(buffer,0,sizeof(buffer));
	        int len = recv(m_iConnectId, buffer, sizeof(buffer),0);///接收
	        if(len > 0){
	        	HandleMsg::GetInstance()->Recieve_Preprocess(buffer,m_iConnectId,len);
	        }
	        else{
	        	exitConnect();
	        	break;
	        }
		}
	}
private:
	bool m_bSendCmdThreadRunning{false};
	bool m_bShouldExit{false};
	int m_iConnectId{0};
};

#endif /* SOCKETMANAGER_H_ */
