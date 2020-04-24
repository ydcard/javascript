/*
 * main.cpp
 *
 *  Created on: Aug 1, 2018
 *      Author: desay-sv
 */

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
#include <chrono>
#include <memory>
#include "SocketManager.h"

#define MYPORT  9090
#define QUEUE   20

int main()
{
	int server_sockfd;
	int conn; // connect 返回值
    server_sockfd = socket(AF_INET,SOCK_STREAM, 0);

    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(MYPORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    // sockfd为需要端口复用的套接字
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

    ///bind，成功返回0，出错返回-1
    if (bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1) {
        perror("bind");
        exit(1);
    }

    printf("port: %d\n",MYPORT);
    ///listen，成功返回0，出错返回-1
    if (listen(server_sockfd,QUEUE) == -1) {
        perror("listen");
        exit(1);
    }

    ///客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

	printf("wait client connect\n");
	///成功返回非负描述字，出错返回-1
	std::shared_ptr<SocketManager> socketMgr = std::make_shared<SocketManager>();
	while (true) {
		conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
		if (conn<0) {
			perror("connect");
			exit(1);
		}
		printf("client connect success: conn is %d\n", conn);

		socketMgr->init(conn);
		printf("reconnect\n");
	}

    printf("close connect！！\n");
    close(server_sockfd);

    return 0;
}

