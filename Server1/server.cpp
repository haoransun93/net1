/*a small file Server
Usage: suppose Server is running on sd1.encs.concordia.ca and server is running on sd2.encs.concordia.ca
.Also suppose there is a file called test.txt on the server.
In the Server,issuse "Server sd2.encs.concordia.ca test.txt size" and you can get the size of the file.
In the Server,issuse "Server sd2.encs.concordia.ca test.txt time" and you can get creation time of the file
*/
#include "server.h"

TcpServer::TcpServer()
{
	WSADATA wsadata;
	if (WSAStartup(0x0202,&wsadata)!=0)
		TcpThread::err_sys("Starting WSAStartup() error\n");
	
	//Display name of local host
	if(gethostname(servername,HOSTNAME_LENGTH)!=0) //get the hostname
		TcpThread::err_sys("Get the host name error,exit");
	
	printf("Server: %s waiting to be contacted for time/size request...\n",servername);
	
	
	//Create the server socket
	if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		TcpThread::err_sys("Create socket error,exit");
	
	//Fill-in Server Port and Address info.
	ServerPort=REQUEST_PORT;
	memset(&ServerAddr, 0, sizeof(ServerAddr));      /* Zero out structure */
	ServerAddr.sin_family = AF_INET;                 /* Internet address family */
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	ServerAddr.sin_port = htons(ServerPort);         /* Local port */
	
	//Bind the server socket
    if (bind(serverSock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0)
		TcpThread::err_sys("Bind socket error,exit");
	
	//Successfull bind, now listen for Server requests.
	if (listen(serverSock, MAXPENDING) < 0)
		TcpThread::err_sys("Listen socket error,exit");

	printf("Successful bind. Listening to server requests now");
}

TcpServer::~TcpServer()
{
	WSACleanup();
}

void TcpServer::start()
{
	while(true) /* Run forever */
	{
		/* Set the size of the result-value parameter */
		clientLen = sizeof(ServerAddr);
		
		/* Wait for a Server to connect */
		if ((clientSock = accept(serverSock, (struct sockaddr *) &ClientAddr, 
			&clientLen)) < 0)
			TcpThread::err_sys("Accept Failed ,exit");
		
        /* Create a Thread for this new connection and run*/
		TcpThread * pt=new TcpThread(clientSock);
		pt->start();
	}
}

//////////////////////////////TcpThread Class //////////////////////////////////////////
void TcpThread::err_sys(char * fmt,...)
{     
	perror(NULL);
	va_list args;
	va_start(args,fmt);
	fprintf(stderr,"error: ");
	vfprintf(stderr,fmt,args);
	fprintf(stderr,"\n");
	va_end(args);
	exit(1);
}

unsigned long TcpThread::ResolveName(char name[])
{
	struct hostent *host;            /* Structure containing host information */
	
	if ((host = gethostbyname(name)) == NULL)
		err_sys("gethostbyname() failed");
	
	/* Return the binary, network byte ordered address */
	return *((unsigned long *) host->h_addr_list[0]);
}

/*
msg_recv returns the length of bytes in the msg_ptr->buffer,which have been recevied successfully.
*/
int TcpThread::msg_recv(int sock,Msg * msg_ptr)
{
	int rbytes,n;
	
	for(rbytes=0;rbytes<MSGHDRSIZE;rbytes+=n)
		if((n=recv(sock,(char *)msg_ptr+rbytes,MSGHDRSIZE-rbytes,0))<=0)
			err_sys("Recv MSGHDR Error");
		
		for(rbytes=0;rbytes<msg_ptr->length;rbytes+=n)
			if((n=recv(sock,(char *)msg_ptr->buffer+rbytes,msg_ptr->length-rbytes,0))<=0)
				err_sys( "Recevier Buffer Error");
			
			return msg_ptr->length;
}

/* msg_send returns the length of bytes in msg_ptr->buffer,which have been sent out successfully
*/
int TcpThread::msg_send(int sock,Msg * msg_ptr)
{
	int n;
	if((n=send(sock,(char *)msg_ptr,MSGHDRSIZE+msg_ptr->length,0))!=(MSGHDRSIZE+msg_ptr->length))
		err_sys("Send MSGHDRSIZE+length Error");
	return (n-MSGHDRSIZE);
	
}

void TcpThread::run() //cs: Server socket
{
	while (true) {
		server_receive_msg();

		//HERE IS THE CODE FOR DECIDING WHAT TO DO
		if (reqp->cmd == QUIT)
			server_cmd_file_quit();

		else if (reqp->cmd == CHECK_FILE_NAME){
			server_cmd_file_name();
			server_send_msg();
		}

		else if (reqp->cmd == LIST)
			server_cmd_file_list();

		else if (reqp->cmd == GET)
			server_cmd_file_get();

		else if (reqp->cmd == PUT)
			server_cmd_file_put();
	}
}

void TcpThread::server_cmd_file_name()
{
	FILE * file;
	fopen_s(&file, reqp->filename, "r");

	if (file == NULL){
		sprintf_s(resp.response, "FILE DOES NOT EXISTS \r\n");
	}
	else {
		sprintf_s(resp.response, reqp->filename);
		sprintf_s(resp.response, "FILE EXISTS \r\n");
		sprintf_s(get_filename, reqp->filename);
		fclose(file);
	}

	resp.cmd = CHECK_FILE_NAME;
	resp.stt = SEND_COMPLETE;
	printf("CONFIRM FILE ");
	printf(reqp->filename);
	printf(" CAN BE TRANSFERED \r\n");
	printf(resp.response);	
}

void TcpThread::server_cmd_file_list()
{
	printf("LIST OF FILES ");
}

void TcpThread::server_cmd_file_put()
{
	FILE * file;
	fopen_s(&file, "transfer_post.txt", "a");
	fputs(reqp->response, file);
	fclose(file);

	if (reqp->stt == SEND_COMPLETE){
		resp.cmd = SERVER_RESET;
		resp.stt = SEND_COMPLETE;
	}
	else {
		resp.stt = SEND;
		resp.cmd = PUT;
	}
	server_send_msg();
}

void TcpThread::server_cmd_file_get()
{
	printf("GET FROM SERVER ");
	FILE * file;
	printf(reqp->filename);
	fopen_s(&file, get_filename, "r");
	if (file == NULL)
	{
		printf("FILE IS BROKEN");
	}
	while (fgets(resp.response, sizeof(resp.response), file)){
		resp.cmd = GET;
		if (feof(file)) {
			resp.stt = SEND_COMPLETE;
			server_send_msg();
			return;
		}
		else {
			resp.stt = SEND;
			server_send_msg();
		}
		server_receive_msg();
	}
	resp.stt = SEND_COMPLETE;
	server_send_msg();
	return;
}

void TcpThread::server_cmd_file_quit()
{
	printf("ENDING CONNECTION WITH THIS HOST");
	resp.cmd = QUIT;
	resp.stt = SEND_COMPLETE;
	sprintf_s(resp.response, "SERVER IS CLOSING SOCKET");
	server_send_msg();
	closesocket(cs); 
}

void TcpThread::server_receive_msg()
{
	if (msg_recv(cs, &rmsg) != rmsg.length)
		err_sys("Receive Req error,exit");

	reqp = (Req *)rmsg.buffer;
	printf("Receive a request from client:%s\n", reqp->hostname);
}

void TcpThread::server_send_msg()
{
	smsg.type = RESP;
	smsg.length = sizeof(Resp);

	memcpy(smsg.buffer, &resp, sizeof(resp));
	if (msg_send(cs, &smsg) != smsg.length)
		err_sys("send Respose failed,exit");
	printf("Response for %s has been sent out \n\n\n", reqp->hostname);
}



////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	TcpServer ts;
	ts.start();
	
	return 0;
}


