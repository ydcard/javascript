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
#include <chrono>
#include <unistd.h>
#include "handle_msg.h"

class SocketManager
{
public:
	SocketManager(){}
	~SocketManager(){}

	void exitConnect();

	void SendCmdToTboxThread();

	void RevMsgFromTboxThread();

	void init(int _connectId);

private:
	bool m_bSendCmdThreadRunning{false};
	bool m_bShouldExit{false};
	int m_iConnectId{0};
};

#endif /* SOCKETMANAGER_H_ */
