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
#include "handle_msg.h"

#define MYPORT  9090
#define QUEUE   20

int server_sockfd;
int conn;
int Yflag;
int Dflag;

void SendMsgToTboxThread(int conn)
{
	while(1)
	{
	    char *buffer = new char[BUFFER_SIZE];
	    memset(buffer,0,sizeof(buffer));
		HandleMsg *handleRecv = new HandleMsg;
		int plen;
		char s[100];
		scanf("%s",s);
		if(strncmp(s,"0",1)!=0){
			printf("please enter 0 to begin send message\n");
			continue;
		}
        handleRecv->HandleSend(buffer,&plen);
        int len = send(conn, buffer, plen, 0);;///接收
//        if(len == -1){
//         	Yflag = 1;
//        }
        if(Yflag == 1){
            printf("关闭连接-Send！！\n");
            close(conn);
            delete buffer;
            delete handleRecv;
            buffer = NULL;
            handleRecv = NULL;
         	break;
        }
        delete buffer;
        delete handleRecv;
        buffer = NULL;
        handleRecv = NULL;
	}
}

void RevMsgFromTboxThread(int conn)
{
	while(1)
	{
	    char buffer[BUFFER_SIZE];
	    memset(buffer,0,sizeof(buffer));
		HandleMsg *handleRecv = new HandleMsg;
        int len = recv(conn, buffer, sizeof(buffer),0);///接收
//        ++Dflag;
//        printf("收到第%d条消息\n",Dflag);
        if(len > 0){
//        	printf("buffer is %s\n",buffer);
//        	printf("len is %d\n",len);
            handleRecv->Recieve_Preprocess(buffer,conn,len);
        }
        else{
           	Yflag = 1;
        }
        if(Yflag == 1){
            printf("关闭连接-Rev！！\n");
            close(conn);
            delete handleRecv;
            handleRecv = NULL;
        	break;
        }
        delete handleRecv;
        handleRecv = NULL;
	}
}

void createSendThr(int conn)
{
	std::thread StartSendToTbox(SendMsgToTboxThread, conn);
	StartSendToTbox.detach();
}

void createReceThr(int conn)
{
	Dflag = 0;
	std::thread StartRecFromTbox(RevMsgFromTboxThread, conn);
	StartRecFromTbox.detach();
}

int main()
{
	Yflag = 0;
    ///定义sockfd
    server_sockfd = socket(AF_INET,SOCK_STREAM, 0);

    ///定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(MYPORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    // sockfd为需要端口复用的套接字
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

    ///bind，成功返回0，出错返回-1
    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        perror("bind");
        exit(1);
    }

    printf("监听%d端口\n",MYPORT);
    ///listen，成功返回0，出错返回-1
    if(listen(server_sockfd,QUEUE) == -1)
    {
        perror("listen");
        exit(1);
    }

    ///客户端套接字
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

	printf("等待客户端连接\n");
	///成功返回非负描述字，出错返回-1
	while(1){
		Yflag = 0;
		conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
		if(conn<0)
		{
			perror("connect");
			exit(1);
		}
		printf("客户端成功连接\n");

		createSendThr(conn);
		createReceThr(conn);

		while(Yflag == 0)
		{
		}
	}

    printf("关闭连接！！\n");
    close(conn);
    close(server_sockfd);

    return 0;
}

