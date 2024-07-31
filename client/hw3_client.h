#ifndef HW3_CLIENT_H
#define HW3_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#define BUF_SIZE 30
#define MAX_FILE 30
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

typedef struct{
	unsigned int size;
	char ftitle[30];
	int is_dir;
}upload_files;


void read_dir(client_info *cl, int sd);
void error_handling(char *message);
void change_directory(client_info *cl, int sd);
void print_dir(client_info cl);
void download_file(client_info *cl, int sd);
void upload_file(client_info *cl, int sd);

#endif
