
#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 40
#define FILENAME_LENGTH 20
#define REQUEST_PORT 5001
#define BUFFER_LENGTH 1024 
#define MAXPENDING 10
#define MSGHDRSIZE 8 //Message Header Size
#define CMD_SIZE sizeof(unsigned short)

enum Command {
	START,

	CHECK_FILE_NAME,
	PUT,
	GET,

	QUIT,
	LIST, 
};

enum TransferState {

	SEND,
	SEND_COMPLETE,
};

typedef enum{
	REQ_SIZE = 1, REQ_TIME, RESP //Message type
} Type;

typedef struct
{
	char hostname[HOSTNAME_LENGTH];
	char filename[FILENAME_LENGTH];
	Command cmd;
	TransferState stt;

} Req;  //request

typedef struct
{
	char response[RESP_LENGTH];
	Command cmd;
	TransferState stt;

} Resp; //response


typedef struct
{
	Type type;
	int  length; //length of effective bytes in the buffer
	char buffer[BUFFER_LENGTH];
} Msg; //message format used for sending and receiving
