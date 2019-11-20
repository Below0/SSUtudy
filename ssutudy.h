#define FDCNT 30

typedef enum _p_type{
	REG = 0,
	LOGIN,
	S_BOOK,
	S_CANCEL,
	S_INFO,
	P_INFO,
	P_WRITE,
	C_INFO,
	C_WRITE,
	RESULT,
	MESSAGE,
	BOARD,
	DISCONNECT
}p_type;

typedef struct {
	int fd;
	char ip[20];
}CLIENT;

typedef struct{
char ID[9];
char PW[17];
char Email[17];
char Name[9];
}USERID;

typedef struct{
    char DATE[9];
    char ID[9];
    int E_TIME;
    int S_TIME;
    int room;
}S_DATA;
typedef struct {
	char Target[5]; // �Խñۿ� ���� ���� ����.
	char ID[9]; // User ID ����
	char Title[31]; // ����
	char Content[257]; // �Խñ� ���� ����.
}COMMUNITY;
typedef struct {
	char Send_ID[9]; // ���� ����� ID ����.
	char Recv_ID[9]; // �޴� ����� ID ����.
	char Content[257]; // ���� ���� ����.
}COM_MSG;
#define PROTO_ID 10;
#define PROTO_PW 18;
#define PROTO_EMAIL 18;
#define PROTO_NAME 10;
