
#include "../Server1/protocol.h"
#pragma comment(lib, "wsock32.lib")
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <iostream>

class TcpClient
{
	int sock;                    /* Socket descriptor */
	struct sockaddr_in ServAddr; /* server socket address */
	unsigned short ServPort;     /* server port */
	Req req;               /* request */
	Resp * respp;          /* pointer to response*/
	Msg smsg, rmsg;               /* receive_message and send_message */
	WSADATA wsadata;
	char * servername;
	char * filename;
	char * type;
	bool transferring = false;
	Command state;

	//char * filename;

public:
	TcpClient(){}
	void run(int argc, char * argv[]);
	~TcpClient();
	int msg_recv(int, Msg *);
	int msg_send(int, Msg *);
	unsigned long resolve_name(char name[]);
	void err_sys(char * fmt, ...);
	void client_connection(char *);
	void client_socket();
	void client_close();
	void client_init();
	void client_msg_send();
	void client_msg_receive();

	int user_input_cmd();
	int user_input_filename();
	bool transfer();

	void client_cmd_list();
	void client_cmd_get();
	void client_cmd_put();
	void client_cmd_quit();

};