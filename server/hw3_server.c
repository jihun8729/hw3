#include "hw3_server.h"

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;
	pthread_t thread_id;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	getcwd(default_dir,sizeof(default_dir)); //디폴트 절대경로 받아오기

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	
	while (1){
		adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
		client_info *cl = (client_info *)malloc(sizeof(client_info));
		cl->sockfd=clnt_sock; // 클라이언트 socket 디스크립터 할당
		printf("connected client: %d \n", clnt_sock); 
		pthread_create(&(thread_id),NULL,handler_client,(void*)cl); // thread로 관리
		pthread_detach(thread_id);
	}

	close(serv_sock);
	return 0;
}




void file_read(client_info *cl){
    DIR *dir = opendir(cl->dir);
	struct dirent *dp;
	int count=0;
	FILE *fp;

	if(dir){
        while((dp = readdir(dir))!=NULL){
        	if(strcmp(dp->d_name,".")==0||strcmp(dp->d_name,"..")==0) //. .. 스킵
                continue;
            strcpy(cl->files[count].ftitle,dp->d_name); //파일 이름 
            fp = fopen(cl->files[count].ftitle,"rb");

			//파일 크기 확인 위해 fseek와 ftell사용
            fseek(fp,0,SEEK_END); 
            cl->files[count].size = ftell(fp);

			//파일인지 디렉토리인지 구분
			if(dp->d_type == DT_DIR){
				cl->files[count].is_dir =1;
			}else{
				cl->files[count].is_dir = 0;
			}

            fclose(fp);
            count++; //파일 개수 증가
        }

		cl->f_count=count;
        closedir(dir);
    }else{
        error_handling("directory can't open");
    } 
}

void change_directory(client_info *cl){
	char mv_directory[BUF_SIZE];
	int str_len;
	char result[50];// cd 결과

	memset(mv_directory,0,sizeof(mv_directory));
	str_len = read(cl->sockfd,mv_directory,BUF_SIZE-1);
	mv_directory[str_len-1] = '\0';
	
	if(access(mv_directory, R_OK|W_OK)!=0){ //해당 디렉토리를 읽을 수 있는지 없는지 판별
		strcpy(result,"이동 불가 디렉토리");
		write(cl->sockfd,result,sizeof(result));
		file_send(cl);
	}else{
		if(chdir(mv_directory) == 0) { // 클라이언트가 이동하길 원하는 디렉토리로 이동
        	if(getcwd(cl->dir, sizeof(cl->dir)) == NULL) { // 해당 디렉토리 경로 받아오기
            	error_handling("getcwd() error!");
       		}

			strcpy(result,"이동 완료!");
			write(cl->sockfd,result,sizeof(result) );

			file_read(cl); 
			file_send(cl);
    	}
	}
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}

void file_send(client_info *c_info){
	write(c_info->sockfd,&c_info->f_count,sizeof(c_info->f_count)); //파일 개수 전달
	write(c_info->sockfd,&c_info->dir,sizeof(c_info->dir)); // 디렉토리 주소 전달
	
	for(int i=0; i<c_info->f_count; i++){
		send(c_info->sockfd,&c_info->files[i],sizeof(c_info->files[i]),0);
	}
}

void *handler_client(void *arg){
	client_info *cl = (client_info *)arg;
	char command[BUF_SIZE];
	int str_len;
	strcpy(cl->dir,default_dir);
	chdir((cl->dir));
	file_read(cl);
	file_send(cl);
	
	while(1){
		str_len = read(cl->sockfd,command,BUF_SIZE); // 실행 할 명령어 수신
		chdir(cl->dir);
		
		if (str_len == 0){    // close request!	
			printf("closed client: %d \n", cl->sockfd);
			close(cl->sockfd);
			free(cl);
			break;
		}
		
		command[str_len]='\0';
		char cs = command[0];
		
		switch(cs){
		 	case 'c':
		 		change_directory(cl);
		 		break;
		 	case 'd':
		 		download_file(cl);
		 		break;
		 	case 'u':
		 		upload_file(cl);
		 		break;
		 	default:
		 		printf("Invalid command\n");
		 		break;
		}
	}
}

void download_file(client_info *c_info){
	int num;
	int read_cnt;
	char buffer[BUF_SIZE];
	FILE *fp;
	int can_download;
	
	read(c_info->sockfd,&num,sizeof(int)); //복사 할 파일 인자값 read
    
	if(access(c_info->files[num].ftitle,R_OK)!=0){ //파일을 읽을 수 있는지 확인
		can_download=0;
		send(c_info->sockfd,&can_download,sizeof(int),0);
	}else{
		can_download=1;
		send(c_info->sockfd,&can_download,sizeof(int),0);
		fp = fopen(c_info->files[num].ftitle,"rb"); //해당 파일 rb모드로 open
		
		while(1){
			memset(buffer, 0, BUF_SIZE); //버퍼 초기화
			read_cnt = fread((void*)buffer, 1, BUF_SIZE, fp);
			if (read_cnt < BUF_SIZE){
				write(c_info->sockfd, buffer, read_cnt);
				break;
			}
			write(c_info->sockfd, buffer, BUF_SIZE);
		}
		fclose(fp);
	} 
}

void upload_file(client_info  *c_info){
	FILE *fp;
	int write_cnt=0;
	int read_cnt;
	f_info up_file;
	char buffer[BUF_SIZE];
	
	if(read(c_info->sockfd,&up_file,sizeof(up_file))<=0){
		error_handling("수신 실패");
	}

	fp = fopen(up_file.ftitle,"wb");
	while (write_cnt<up_file.size){
        memset(buffer, 0, BUF_SIZE); //버퍼 초기화
        read_cnt = read(c_info->sockfd, buffer, BUF_SIZE);

        if(read_cnt<0) break;
		
        fwrite((void*)buffer, 1, read_cnt, fp);
        write_cnt+=read_cnt;
    } 

	fclose(fp);
	c_info->f_count++;
}
