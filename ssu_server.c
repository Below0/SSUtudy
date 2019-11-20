#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "ssutudy.h"
#include<mysql.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>

#define DB_HOST "127.0.0.1"
#define DB_USER "np"
#define DB_PASS "np123"
#define DB_NAME "signup"
#define CHOP(x) x[strlen(x) - 1] = ' '

MYSQL *connection = NULL, conn;//DB 연결했을 때 연결 다루는 구조체

int clientCount = 0;
int sockfd_connect[FDCNT];
char User_connect[FDCNT][9] = { }; // <추가>

void *client_main(CLIENT *client);
int client_login(USERID uid);
int client_reg(USERID uid);
int s_book(int);
int s_cancel(int);
int s_info(int);
int send_res(int, int);

pthread_mutex_t mutex;

int p_Info(int);
int p_Write(int);
int c_Info(int);
int c_Write(int);
int s_Msg(int);
int load_Board(int);



int main(int argc, char*argv[]){

	//MYSQL *connection = NULL, conn;//DB 연결했을 때 연결 다루는 구조체
	//MYSQL_RES *sql_result;//쿼리 내렸을 때 그 결과 다루기 위한 구조체
	MYSQL_ROW sql_row;//데이터의 하나의 ROW 가르킴. 없으면 NULL 
	//int query_stat;
	//char query[255];
	
		mysql_init(&conn);//mysql 연결 초기화
	connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
		
		if (connection == NULL) {
			fprintf(stderr, "MYSQL CONNECTION ERROR : %s", mysql_error(&conn));
				return 1;
		}

	int i;
	int tempfd = 0;
	int clnt_addr_size = 0;
	int sockfd_listen;
	struct sockaddr_in server;
	struct sockaddr_in clnt_addr;
	CLIENT client_data[FDCNT] = {0,};
	pthread_mutex_init(&mutex, NULL);

	pthread_t ptid[FDCNT] = {0,};

	memset(&server, 0, sizeof(server));
	server.sin_family = PF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(argv[1])); // port #

	if((sockfd_listen = socket(PF_INET,SOCK_STREAM,0)) == -1){
		printf("making listen sock error!\n");
		return 1;

	}
	int option = 1;
	setsockopt(sockfd_listen, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if(bind(sockfd_listen, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
		printf("bind() error!\n");
		return 1;
	}

	if(listen(sockfd_listen, 10) == -1){
		printf("fail to call listen()\n");
		return 1;
	}
	clnt_addr_size = sizeof(clnt_addr);
	printf("Server on\n");
	while(1){
		tempfd = accept(sockfd_listen, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
		if(clientCount == FDCNT){
			printf("client is full\n");
			close(tempfd);
			continue;
		}
		if(tempfd < 0){
			printf("fail to connect!\n");
			continue;

		}
		for(i = 0; i <FDCNT;i++){
			if(client_data[i].fd == 0){
				client_data[i].fd = tempfd;
				break;
			}
		}
		strcpy(client_data[i].ip, inet_ntoa(clnt_addr.sin_addr));
		pthread_create(ptid+i, NULL, (void*)client_main,client_data+i);
		pthread_detach(ptid[i]);
		clientCount++;
		printf("accepted sock : %d. clnt cnt : %d\n", client_data[i].fd, clientCount);




	}
	pthread_mutex_destroy(&mutex);
	close(sockfd_listen);

	return 0;
}

void * client_main(CLIENT *client){
	printf("%s is connected\n",client->ip);
	int clnt_sock = client->fd;
	char msg[200] = "connected!";
	char buff[256];
	p_type pt;
	USERID uid;
	int isConnect = 1;
	int login_check = -1;
	while(1){
		isConnect = recv(clnt_sock,&pt, sizeof(pt),0);

		printf("%s) MSG type : %d\n", client->ip,pt);
		if(pt == DISCONNECT || isConnect < 1){
		if(isConnect < 1) printf("errno : %d\n", errno);
			break;
		}
		if(pt > 12 || pt < 0) continue;
		switch(pt){

			case LOGIN:
				read(clnt_sock, uid.ID, sizeof(uid.ID)-1);
				read(clnt_sock, uid.PW, sizeof(uid.PW)-1);
				login_check = send_res(clnt_sock,client_login(uid));

				//search DB Input userID

				//로그인 성공시 로그인 된 ID를 알기위해 사용. <추가>
				sockfd_connect[clientCount] = clnt_sock;
				memcpy(User_connect[clientCount], uid.ID, sizeof(uid.ID) - 1);

				break;

			case REG:
				read(clnt_sock, uid.ID, 8);
				read(clnt_sock, uid.PW, 16);
				read(clnt_sock, uid.Email, 16);
				read(clnt_sock, uid.Name, 8);
				send_res(clnt_sock,client_reg(uid)); 
				break;

			case S_BOOK:
				send_res(clnt_sock,s_book(clnt_sock));
				break;
			case S_CANCEL:
				send_res(clnt_sock,s_cancel(clnt_sock));

				break;
			case S_INFO:
				send_res(clnt_sock,s_info(clnt_sock));

				break;
					/*	case P_INFO:
						send_res(clnt_sock, p_Info(clnt_sock));

						break;*/
						case P_WRITE:
						send_res(clnt_sock, p_Write(clnt_sock));
						break;
				/*case C_INFO:
				//send_res(clnt_sock, c_Info(clnt_sock));
				//			break;
				case C_WRITE:
				send_res(clnt_sock, c_Write(clnt_sock));
				break;*/
			case MESSAGE:
				send_res(clnt_sock, s_Msg(clnt_sock));
				break;
			case BOARD:
				send_res(clnt_sock, load_Board(clnt_sock));
				break;
		}

	}


	close(clnt_sock);
	pthread_mutex_lock(&mutex);
	printf("%s disconnect!\n",client->ip);
	clientCount--;
	pthread_mutex_unlock(&mutex);
	client->fd = 0;
}

int client_login(USERID uid){
	char query[255];
		char id[20];
		int query_stat;
		MYSQL_RES *sql_result;
		MYSQL_ROW sql_row;

		//해당 uid.ID 와 uid.PW와 매칭되는 계정이 DB에 있을 경우
		// 없을 경우 return -1
		
		sprintf(query, "select * from reg where id = '%s'", uid.ID);
		query_stat = mysql_query(connection, query);
		sql_result = mysql_store_result(connection);
		sql_row = mysql_fetch_row(sql_result);
		
		if (sql_row == NULL) {
			printf("ID not exist\n");
			return -1;
		}
		else {
			if (strcmp(sql_row[1], uid.PW) == 0)
			{
				printf("login complete\n");
					return 1;
			}
			else
			{
				printf("login fail\n");
					return -1;
			}
		}
}

int client_reg(USERID uid){
	char query[255];
		int query_stat;
		printf("-------------REG--------------\n");
		printf("id : %s pw : %s\n", uid.ID, uid.PW);
		printf("email : %s@soongsil.ac.kr name : %s\n", uid.Email, uid.Name);
		printf("------------------------------\n");
		printf("result : ");
		//if id already in DB
		// return -1
		//USERID store in DB

		sprintf(query, "insert into reg values" "('%s', '%s', '%s', '%s')", uid.ID, uid.PW, uid.Email, uid.Name);

	query_stat = mysql_query(connection, query);
		if (query_stat != 0)
		{
			fprintf(stderr, "mysql query error : %s", mysql_error(&conn));
				printf("\nID is alreay exist\n");
				return -1;
		}
	
		//mysql_close(connection);
		return 1;
}

int s_book(int clnt_sock){
	char query[255];
	char query_insert[255];
	char e_time[10];
	char s_time[10];
	char s_room[10];
	int query_stat;

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;
	S_DATA sif;
	//connection = NULL;
	read(clnt_sock, sif.DATE, 8);
	sif.DATE[8] = 0;
	read(clnt_sock, sif.ID, 8);
	sif.ID[8] = 0;
	read(clnt_sock, (void*)&sif.E_TIME, 4);
	read(clnt_sock, (void*)&sif.S_TIME, 4); 
	read(clnt_sock, (void*)&sif.room, 4);
	printf("-------------BOOK--------------\n");
	printf("DATE : %s ID : %s \n Start : %d End : %d room_num : %d\n", sif.DATE, sif.ID, sif.S_TIME, sif.E_TIME, sif.room);
	printf("-------------------------------\n");

	sprintf(e_time, "%d", sif.E_TIME);
	sprintf(s_time, "%d", sif.S_TIME);
	sprintf(s_room, "%d", sif.room);

	// pthread_mutex_lock(&mutex);
	//check DB에 이미 존재할 경우에도 -1
	//db store falied -> return -1
	sprintf(query, "select * from book where date = '%s'", sif.DATE);
	query_stat = mysql_query(connection, query);
	sql_result = mysql_store_result(connection);
	if ((sql_row = mysql_fetch_row(sql_result)) == NULL) {
		sprintf(query_insert, "insert into book values""('%s', '%s', '%s', '%s', '%s')", sif.DATE, sif.ID, e_time, s_time, s_room);
		query_stat = mysql_query(connection, query_insert);
		if (query_stat != 0) {
			fprintf(stderr, "mysql query error : %s", mysql_error(&conn));
			printf("\ndb stored failed\n");
			return -1;
		}
		//mysql_close(connection);
	}
	else {
		do{
			if ((strcmp(sql_row[0], sif.DATE) == 0 &&
						strcmp(sql_row[2], e_time) == 0 &&
						strcmp(sql_row[4], s_room) == 0) ||
					(strcmp(sql_row[0], sif.DATE) == 0 &&
					 strcmp(sql_row[3], s_time) == 0 &&
					 strcmp(sql_row[4], s_room) == 0)) {
				printf("already exist in db\n");
				return -1;
			}
		}while((sql_row = mysql_fetch_row(sql_result)) != NULL);
		//else{
		sprintf(query, "insert into book values""('%s', '%s', '%d', '%d', '%d')", sif.DATE, sif.ID, sif.E_TIME, sif.S_TIME, sif.room);
		query_stat = mysql_query(connection, query);
		if (query_stat != 0) {
			fprintf(stderr, "mysql query error : %s", mysql_error(&conn));
			printf("db stored failed\n");
			return -1;
		}
		//mysql_close(connection);
		//return 1;
		//}
	}

	// pthread_mutex_unlock(&mutex);
	return 1;
}

int s_cancel(int clnt_sock){
	char query[255];
	char query_delete[255];
	char e_time[10];
	char s_time[10];
	char s_room[10];
	int query_stat;
	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;

	S_DATA sif;
	read(clnt_sock, sif.DATE, 8);
	sif.DATE[8] = 0;
	read(clnt_sock, sif.ID, 8);
	sif.ID[8] = 0;
	read(clnt_sock, (void*)&sif.E_TIME, 4);
	read(clnt_sock, (void*)&sif.S_TIME, 4);
	read(clnt_sock, (void*)&sif.room, 4);
	printf("%s %s %d %d %d\n", sif.DATE, sif.ID, sif.E_TIME, sif.S_TIME, sif.room);

	sprintf(e_time, "%d", sif.E_TIME);
	sprintf(s_time, "%d", sif.S_TIME);
	sprintf(s_room, "%d", sif.room);
	//pthread_mutex_lock(&mutex);
	//check DB에 해당 시간이 없을 경우  -1
	//db 삭제 falied -> return -1
	sprintf(query, "select * from book where date = '%s'", sif.DATE);
	query_stat = mysql_query(connection, query);
	sql_result = mysql_store_result(connection);

	printf("test1\n");

	if ((sql_row = mysql_fetch_row(sql_result)) == NULL) {
		printf("\ndate is not exist\n");
		//mysql_close(connection);
		return -1;
	}

	else {
		printf("test2\n");
		do {
			printf("test3\n");
			printf("\n%s\n", sql_row[2]);
			printf("%s\n", sql_row[3]);
			printf("%s\n", sql_row[4]);
			if (strcmp(sql_row[0], sif.DATE) == 0 &&
		         	strcmp(sql_row[2], e_time) == 0 &&
				strcmp(sql_row[3], s_time) == 0 &&
				strcmp(sql_row[4], s_room) == 0) {
				printf("\ndebugging\n");
				sprintf(query_delete, "delete from book where date = '%s' AND etime = '%s' AND stime = '%s' AND room = '%s'", sif.DATE, e_time, s_time, s_room);
					//sprintf(query_delete, "delete from book where date = '%s'", sif.DATE);
					query_stat = mysql_query(connection, query_delete);
					if (query_stat != 0) {
						fprintf(stderr, "mysql query error : %s", mysql_error(&conn));
						printf("\ndelete failed\n");
						return -1;
					}
				//break;
				return 1;
			}
		}while ((sql_row = mysql_fetch_row(sql_result)) != NULL);
		//mysql_close(connection);
	}
	return 1;
}

int s_info(int clnt_sock){
	char query[255];
	char date[9];
	char schedule[10];
	int query_stat;

	MYSQL_RES *sql_result;
	MYSQL_ROW sql_row;

	read(clnt_sock, date, 8);

	memset(query, '0', sizeof(query));
	sprintf(query, "select * from book where date = '%s'", date);
	query_stat = mysql_query(connection, query);
	sql_result = mysql_store_result(connection);
	if ((sql_row = mysql_fetch_row(sql_result)) == NULL) {
		printf("There is no schedule in that date.\n");
		return -1;
	}

	for (int i = 1; i < 5; i++) {
		memset(query, '0', sizeof(query));
		sprintf(query, "select * from book where date = '%s' AND room = '%d'", date, i);
		query_stat = mysql_query(connection, query);
		sql_result = mysql_store_result(connection);

		memset(schedule, '0', sizeof(schedule));

		if ((sql_row = mysql_fetch_row(sql_result)) == NULL) {
			//printf("There is no schedule in that date.\n");
			schedule[8] = '\r';
			schedule[9] = '\n';
			write(clnt_sock, schedule, 10);
		}
		else {
			int e_num = 0;
			int s_num = 0;
			do {
				e_num = atoi(sql_row[2]);
				s_num = atoi(sql_row[3]);
				schedule[e_num - 1] = '1';
				schedule[s_num - 1] = '1';
				schedule[8] = '\r';
				schedule[9] = '\n';
				write(clnt_sock, schedule, 10);
			} while ((sql_row = mysql_fetch_row(sql_result)) != NULL);
		}
	}

	return 1;
}

int send_res(int clnt_sock, int check){
	p_type res = RESULT;
	if(check > 0){
		printf("Success\n");
		write(clnt_sock,"0\r\n", 3);
		return 1;
	}
	else{
		printf("Failure\n");
		write(clnt_sock, "1\r\n", 3);
		return -1;
	}
	return -1;
}

/*int p_Info(int clnt_sock) {
  COMMUNITY post;
  int DB_len; // 게시글 이나 댓글 있는 만큼 다 읽어서 가져오기 위해 사용.
//
// DB에 저장되어 있는 모든 게시글을 읽어옴.
//
for (int i = 0; i < DB_len; i++) {
write(clnt_sock, post.Target, 4);
write(clnt_sock, post.ID, 8);
write(clnt_sock, post.Content, 256);
}
return 1;
}
*/
int p_Write(int clnt_sock) {
COMMUNITY post;

int tmp;
tmp = read(clnt_sock, post.ID, 8);
if(tmp < 1) return -1;
post.ID[tmp] = 0;
tmp = read(clnt_sock, post.Title, 30);if(tmp < 1) return -1;

post.Title[16] = 0;
tmp = read(clnt_sock, post.Content, 256);if(tmp < 1) return -1;

post.Content[tmp] = 0;


printf("--------------POST----------------\n");
printf("ID : %s\nTITLE : %s\nContent : \n %s\n", post.ID, post.Title, post.Content);
printf("----------------------------------\n");
//   DB에 저장할때 Target에 DB에 저장되는 위치 정보를 저장.
//

return 1;

}

/*
   int c_Info(int clnt_sock) {
   COMMUNITY comment;
   int DB_len; // 게시글 이나 댓글 있는 만큼 다 읽어서 가져오기 위해 사용.
   for (int i = 0; i < DB_len; i++) {
   write(clnt_sock, comment[i].Target, strlen(comment[i].Target));
   write(clnt_sock, comment[i].ID, strlen(comment[i].ID));
   write(clnt_sock, comment[i].Content, strlen(comment[i].Content));
   }
   return 1;
   }
   int c_Write(int clnt_sock) {
   COMMUNITY comment;

   read(clnt_sock, comment.ID, 8);
   read(clnt_sock, comment.Content, 256);

//
//   DB에 저장할때 Target에 해당 게시물에 대한 정보를 저장.
//
return 1;
}
*/

int load_Board(int clnt_sock) {
	COMMUNITY board;
	COMMUNITY comment;
	char title[18];
	char w_id[10];
	char content[258];

	read(clnt_sock, board.Target,4);
	//
	//   Target에 해당 되는 게시물에 대한 글과 쓰인 댓글 불러오기.
	//
	//write(clnt_sock,)

}
int s_Msg(int clnt_sock) {
	COM_MSG com_msg;
	int cnt = 0;
	char s_ID[10];
	char r_ID[10];
	char content[258];

	read(clnt_sock, com_msg.Send_ID, 8);
	read(clnt_sock, com_msg.Recv_ID, 8);
	read(clnt_sock, com_msg.Content, 256);

	memcpy(s_ID, com_msg.Send_ID, 9);
	strcat(s_ID, "\r\n");
	memcpy(r_ID, com_msg.Recv_ID, 9);
	strcat(r_ID, "\r\n");
	memcpy(content, com_msg.Content, 257);
	strcat(content, "\r\n");

	//  DB에 보낸 사람의 ID, 받는 사람의 ID, 내용 저장.

	//if() 연결된 소켓의 리스트를 통해 받는 사람의 ID가 연결되어 있는 소켓이라면 메시지 전송 아니면 끝
	while (cnt < FDCNT) {
		if (!strcmp(com_msg.Recv_ID, User_connect[cnt])) {
			clnt_sock = sockfd_connect[cnt];
			write(clnt_sock, s_ID, 11);
			write(clnt_sock, r_ID, 11);
			write(clnt_sock, content, 259);
			break;
		}
		cnt++;
	}
	return 1;
}
