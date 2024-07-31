#ifndef HW3_SERVER_H
#define HW3_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include <dirent.h>
#define BUF_SIZE 30
#define MAX_FILE 30



char default_dir[30];

typedef struct{
    unsigned int size;
    char ftitle[30];
    int is_dir;
}f_info;

typedef struct{
    int sockfd;
    char dir[50];
    f_info files[MAX_FILE];
	int f_count;
}client_info;




void error_handling(char *buf);
void init_client(client_info *c_info, int sockfd, char *dir); //클라이언트 활성화
void *handler_client(void *arg); // 명령을 받으며 해당 함수 실행시키는 함수
void change_directory(client_info *cl); //디렉토리 변경함수
void file_read(client_info *cl); // 디렉토리 내의 파일 읽기
void file_send(client_info *cl); // 디렉토리 내의 파일 정보 전달
void download_file(client_info *c_info); // 다운로드 받을 파일 정보 전달
void upload_file(client_info  *c_info); // 업로드 할 파일 정보 수신

#endif
