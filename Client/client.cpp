
/*a small file client
Usage : suppose client is running on sd1.encs.concordia.ca and server is running on sd2.encs.concordia.ca
		.Also suppose there is a file called test.txt on the server.
		In the client, issuse "client sd2.encs.concordia.ca test.txt size" and you can get the size of the file.
		In the client, issuse "client sd2.encs.concordia.ca test.txt time" and you can get creation time of the file
		*/

#include "client.h"

using namespace std;

void TcpClient::run(int argc, char * argv[])
{
	servername = argv[1];
	filename = argv[2];
	type = argv[3];
	//init WSA
	client_init();

	//create socket
	client_socket();

	//connect to server
	client_connection(servername); //server name

	while (user_input_cmd() == 1); //loop

	//close the client socket
	client_close();

}

TcpClient::~TcpClient()
{
	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
}

void TcpClient::client_init(){
	//if (argc != 4)
	//err_sys("usage: client servername filename size/time");

	//initilize winsocket
	if (WSAStartup(0x0202, &wsadata) != 0)
	{
		WSACleanup();
		err_sys("Error in starting WSAStartup()\n");
	}

	printf("step 1");
	//Display name of local host and copy it to the req
	if (gethostname(req.hostname, HOSTNAME_LENGTH) != 0) //get the hostname
		err_sys("can not get the host name,program exit");
	printf("%s%s\n", "Client starting at host:", req.hostname);

	strcpy_s(req.filename, filename);
	printf("step 3");
	//request type, can get rid of
	if (strcmp(type, "time") == 0)
		smsg.type = REQ_TIME;
	else if (strcmp(type, "size") == 0)
		smsg.type = REQ_SIZE;
	else err_sys("Wrong request type\n");
}

void TcpClient::client_close()
{	//close the client socket
	closesocket(sock);
}

void TcpClient::client_socket()
{
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) //create the socket 
		err_sys("Socket Creating Error");
}

void TcpClient::client_connection(char * servername)
{
	printf("step 4");
	//connect to the server
	ServPort = REQUEST_PORT;
	memset(&ServAddr, 0, sizeof(ServAddr));     /* Zero out structure */
	ServAddr.sin_family = AF_INET;             /* Internet address family */
	ServAddr.sin_addr.s_addr = resolve_name(servername);   /* Server IP address */
	ServAddr.sin_port = htons(ServPort); /* Server port */
	if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
		err_sys("Socket Creating Error");
}

void TcpClient::client_msg_send()
{
	memcpy(smsg.buffer, &req, sizeof(req)); //copy the request to the msg's buffer

	smsg.length = sizeof(Req);
	smsg.type = RESP;

	//fprintf(stdout, "Send reqest to %s\n", argv[1]);
	if (msg_send(sock, &smsg) != sizeof(req))
		err_sys("Sending req packet error.,exit");
}

int TcpClient::msg_send(int sock, Msg * msg_ptr)
{
	int n;
	if ((n = send(sock, (char *)msg_ptr, MSGHDRSIZE + msg_ptr->length, 0)) != (MSGHDRSIZE + msg_ptr->length))
		err_sys("Send MSGHDRSIZE+length Error");
	return (n - MSGHDRSIZE);
}

/*
RECEIVE
*/
void TcpClient::client_msg_receive()
{
	//receive the response
	if (msg_recv(sock, &rmsg) != rmsg.length)
		err_sys("recv response error,exit");

	respp = (Resp *)rmsg.buffer;
	//printf("Response:%s\n\n\n", respp->response);

}

/*
msg_recv returns the length of bytes in the msg_ptr->buffer,which have been recevied successfully.
*/
int TcpClient::msg_recv(int sock, Msg * msg_ptr)
{
	int rbytes, n;
	for (rbytes = 0; rbytes<MSGHDRSIZE; rbytes += n)
		if ((n = recv(sock, (char *)msg_ptr + rbytes, MSGHDRSIZE - rbytes, 0)) <= 0)
			err_sys("Recv MSGHDR Error");

	for (rbytes = 0; rbytes<msg_ptr->length; rbytes += n)
		if ((n = recv(sock, (char *)msg_ptr->buffer + rbytes, msg_ptr->length - rbytes, 0)) <= 0)
			err_sys("Recevier Buffer Error");
	return msg_ptr->length;
}


/*


*/
unsigned long TcpClient::resolve_name(char name[])
{
	struct hostent *host;            /* Structure containing host information */

	if ((host = gethostbyname(name)) == NULL)
		err_sys("gethostbyname() failed");

	/* Return the binary, network byte ordered address */
	return *((unsigned long *)host->h_addr_list[0]);
}

void TcpClient::err_sys(char * fmt, ...) //from Richard Stevens's source code
{
	perror(NULL);
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	//exit(1);
}

int TcpClient::user_input_cmd()
{
	printf("\r\n");
	printf("0.		QUIT - CLOSE CLIENT AND END THE CONNECTION \r\n");
	printf("1.		GET - TRANSFER FILE FROM SERVER TO CLIENT \r\n");
	printf("2.		PUT - TRANSFER FILE FROM CLIENT TO SERVER \r\n");
	printf("3.		LIST CLIENT - GET A LIST OF FILES ON CLIENT \r\n");
	printf("4.		LIST SERVER - GET A LIST OF FILES ON SERVER \r\n");

	int cmd = 0;
	cin >> cmd;

	//quit
	if (cmd == 0){
		req.stt = SEND;
		req.cmd = QUIT;
		transfer();
		exit(1);
	}

	else if (cmd == 1) {
		//confirm filename
		state = GET;
		while(user_input_filename() == 1);
		//get state
		req.cmd = state;
		req.stt = SEND;
		transfer();
	}
	
	//transfer
	else if (cmd == 2) {
		state = PUT;
		while (user_input_filename() == 1);
		//set state
		transfer();
	}

	//list client
	else if (cmd == 3) {
		//list
		//transfer();
	}
	else if (cmd == 4) {
		//set state
		req.cmd = LIST;
		req.stt = SEND;
		transfer();
	}

	else {
		printf("PLEASE ENTER THE PROPER NUMBER VALUE \r\n");
	}

	return 1;
}

int TcpClient::user_input_filename(){
	req.cmd = CHECK_FILE_NAME;
	req.stt = SEND;

	printf("PLEASE ENTER THE FILE NAME TO BE TRANSFERRED \r\n");
	cin >> req.filename;

	transfer();
	return 0;
}

bool TcpClient::transfer()
{
	transferring = true;
	while (transferring)
	{
		client_msg_send();
		client_msg_receive();

		if (respp->cmd == QUIT){
			printf(respp->response);
		}
		else if (respp->cmd == PUT){
			printf(respp->response);
		}
		else if (respp->cmd == GET){
			client_cmd_get();
		}
		else if (respp->cmd == LIST){
			printf(respp->response);
		}

		if (respp->stt == SEND_COMPLETE){
			transferring = false;
		}
	}

	printf("SERVER TRANSFER COMPLETE \r\n");
	return true;
}

void TcpClient::client_cmd_get()
{
	FILE * file;
	fopen_s(&file, "transfer_get.txt", "a");
	fputs(respp->response, file);
	fclose(file);
	file = NULL;
}

////////////////////////////////////////////

int main(int argc, char *argv[]) //argv[1]=servername argv[2]=filename argv[3]=time/size
{
	TcpClient * tc = new TcpClient();

	argv[1] = "PC";
	argv[2] = "test.txt";
	argv[3] = "size";

	tc->run(4, argv);

	return 0;
}
