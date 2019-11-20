#include<stdio.h>
#include<sys/types.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include"ssutudy.h"
#define BUF_SIZE 1024
#define RLT_SIZE 8
#define OPSZ 4

void error_handling(char *message){
fputs(message,stderr);
fputc('\n',stderr);
exit(1);
}

int main(int argc, char *argv[]) {
int sock;
char opmsg[BUF_SIZE];
char msg[2];
float result[2];
	int opnd_cnt, i;
struct sockaddr_in serv_adr;
if(argc!= 3){
printf("argv error!\n");
return 0;
}
sock = socket(PF_INET, SOCK_STREAM, 0);
if(sock == -1){
	error_handling("connect() error!");

}
memset(&serv_adr, 0, sizeof(serv_adr));
serv_adr.sin_family = PF_INET;//type
serv_adr.sin_addr.s_addr =inet_addr(argv[1]);//IP address
serv_adr.sin_port=htons(atoi(argv[2]));//Port

if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1){
	error_handling("connect() error!");
}
else{
puts("Connected...........");
/*
read(sock, opmsg, 200);
printf("Operation result: %s \n", opmsg);
*/
p_type select;
p_type reply;
USERID uid;
S_DATA sda;
while(select != DISCONNECT){
	printf("0.REG 1.LOGIN 2.S_BOOK 3.S_CANCEL 4.S_INFO -1.disconnect: ");
	scanf("%d",&select);
	write(sock, &select, sizeof(p_type));
	switch(select){
		case LOGIN:
			printf("ID : ");
			scanf("%s", uid.ID);
			printf("PW : ");
			scanf("%s", uid.PW);
			printf("\n");
			write(sock, uid.ID, 8);
			write(sock, uid.PW, 16);

			break;
		case REG:
			printf("ID, PW, email, name\n");
			scanf("%s", uid.ID);
			scanf("%s", uid.PW);
			scanf("%s", uid.Email);
			scanf("%s", uid.Name);
			write(sock, uid.ID, 8);
			write(sock, uid.PW, 16);
			write(sock, uid.Email, 16);
			write(sock, uid.Name, 8);
			break;

		case S_BOOK:
			printf("ID, DATE, E,S\n");
			scanf("%s", sda.ID);
			scanf("%s", sda.DATE);
			scanf("%d", &sda.E_TIME);
			scanf("%d", &sda.S_TIME);
			write(sock, sda.DATE, 8);
			write(sock, sda.ID, 8);
			write(sock, (void*)&sda.E_TIME, 4);
			write(sock, (void*)&sda.S_TIME, 4);


			break;
		case S_CANCEL:
			break;
		case S_INFO:
			break;



	}
}

close(sock);
}
return 0;

}
