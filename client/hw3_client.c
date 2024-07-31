#include "hw3_client.h"


int main(int argc, char *argv[])
{
	int sd;
	client_info cl;
	char command[BUF_SIZE];
	
	char cs;
	struct sockaddr_in serv_adr;
	if (argc != 3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1); 
	}
	
	sd = socket(PF_INET, SOCK_STREAM, 0);   

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

if (connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	else{
		read_dir(&cl,sd);
		puts("Connected...........");
	}
		
	
	while (1) 
	{
		print_dir(cl);
		fputs("디렉토리 이동: c 다운로드: d, 업로드: u\n",stdout);
		fputs("Input message(Q to quit): ", stdout);
		fgets(command, BUF_SIZE, stdin);

		if (!strcmp(command,"q\n") || !strcmp(command,"Q\n"))
			break;
		cs = command[0];
		command[strlen(command)] = '\0';

		write(sd,command,sizeof(command));
		switch(cs){
			case 'c':
				change_directory(&cl,sd);
				break;
			 case 'd':
			 	download_file(&cl,sd);
			 	break;
			 case 'u':
			 	upload_file(&cl,sd);
				break;
			default:
				printf("Invalid command\n");
				break;
		}
		
	}
	close(sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void read_dir(client_info *cl, int sd){
	read(sd,&cl->f_count,sizeof(cl->f_count));
	read(sd,&cl->dir,sizeof(cl->dir));

	for(int i=0; i<cl->f_count; i++){
		read(sd,&cl->files[i],sizeof(cl->files[i]));
	}
}

void print_dir(client_info cl){
	printf("현재 디렉토리 주소: %s\n",cl.dir);
	for(int i=0; i<cl.f_count; i++){
		if(cl.files[i].is_dir==1){
			printf("%d : %s(dir)\n",i+1,cl.files[i].ftitle);
			continue;
		}
		printf("%d : %s(%d)\n",i+1,cl.files[i].ftitle,cl.files[i].size);
	}
}
void change_directory(client_info *cl, int sd){
	char mv_dir[BUF_SIZE];
	char erro[50];

	printf("이동할 디렉토리를 입력하시오: ");
	fgets(mv_dir,sizeof(mv_dir),stdin);
	
	write(sd,mv_dir,strlen(mv_dir));
	read(sd,erro,sizeof(erro));
	printf("%s\n",erro);
	read_dir(cl,sd);
}

void download_file(client_info *cl, int sd){
	int num;
	FILE *fp;
	char input[BUF_SIZE];
	int read_cnt, write_cnt;
	char buffer[BUF_SIZE];

	printf("복사할 파일의 번호를 입력하시오(종료 0번): ");
	fgets(input,sizeof(input),stdin);
	input[strlen(input)]='\0';
	num = atoi(input);

    if(num>cl->f_count) //파일 수보다 큰 번호를 입력할 시 에러
            error_handling("잘못된 파일 번호\n");

    num-=1;
    write(sd,&num,sizeof(int)); //복사 할 파일 번호 전송

    fp = fopen(cl->files[num].ftitle,"wb");
    write_cnt=0;

    while (write_cnt<cl->files[num].size){
        memset(buffer, 0, BUF_SIZE); //버퍼 초기화
        read_cnt = read(sd, buffer, BUF_SIZE);
		
        if(read_cnt<0) break;

        fwrite((void*)buffer, 1, read_cnt, fp);
        write_cnt+=read_cnt;
    }
    puts("Received file data");
	fclose(fp);
}

void upload_file(client_info *cl, int sd){
	DIR *dir = opendir(".");
	struct dirent *dp;
	upload_files up_file[MAX_FILE];
	int count=0;
	FILE *fp;
	int num;
	char input[BUF_SIZE];
	char buffer[BUF_SIZE];
	int read_cnt;

	if(dir){
		while((dp = readdir(dir))!= NULL){
			if(strcmp(dp->d_name,".")==0||strcmp(dp->d_name,"..")==0) //. .. 스킵
                continue;
			
			strcpy(up_file[count].ftitle,dp->d_name); //파일 이름
			fp = fopen(up_file[count].ftitle,"rb");

			//파일 크기 확인 위해 fseek와 ftell사용
            fseek(fp,0,SEEK_END); 
            up_file[count].size = ftell(fp);
			fclose(fp);

			if(dp->d_type == DT_DIR){
				up_file[count].is_dir =1;
			}else{
				up_file[count].is_dir = 0;
			}
        	count++; //파일 개수 증가 
		}
	}

	for(int i=0; i<count; i++){
		printf("%d: %s(%d)\n",i+1,up_file[i].ftitle,up_file[i].size);
	}

	printf("복사 할 파일을 선택하시오: ");
	fgets(input,sizeof(input),stdin);

	input[strlen(input)-1]='\0';
	num = atoi(input);
	num-=1;
	
	
	if(send(sd,&up_file[num],sizeof(up_file[num]),0)<=0){
		error_handling("파일 정보 전송 실패");
	}
	
	fp = fopen(up_file[num].ftitle,"rb");
	while(1){
		memset(buffer, 0, BUF_SIZE); //버퍼 초기화
        read_cnt = fread((void*)buffer, 1, BUF_SIZE, fp);
		
        if (read_cnt < BUF_SIZE){
            write(sd, buffer, read_cnt);
            break;
        }
        write(sd, buffer, BUF_SIZE);
    }
    fclose(fp); 

}
