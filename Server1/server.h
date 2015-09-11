#include "protocol.h"
#pragma comment(lib, "wsock32.lib")
#include <winsock.h>
//#include <iostream.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <process.h>
#include "Thread.h"

#ifndef SER_TCP_H
#define SER_TCP_H

class TcpServer
{
	int serverSock,clientSock;     /* Socket descriptor for server and client*/
	struct sockaddr_in ClientAddr; /* Client address */
	struct sockaddr_in ServerAddr; /* Server address */
	unsigned short ServerPort;     /* Server port */
	int clientLen;            /* Length of Server address data structure */
	char servername[HOSTNAME_LENGTH];

public:
		TcpServer();
		~TcpServer();
		void TcpServer::start();
};

class TcpThread :public Thread
{
	int cs;
	Msg rmsg, smsg;
	Resp resp;
	Req * reqp;

	char get_filename[20];
public: 
	TcpThread(int clientsocket):cs(clientsocket)
	{}
	virtual void run();
    int msg_recv(int ,Msg * );
	int msg_send(int ,Msg * );
	unsigned long ResolveName(char name[]);
    static void err_sys(char * fmt,...);

	void server_receive_msg();
	void server_send_msg();

	void server_cmd_file_list();
	void server_cmd_file_name();
	void server_cmd_file_get();
	void server_cmd_file_put();
	void server_cmd_file_quit();
};

#endif