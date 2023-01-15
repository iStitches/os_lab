#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <wait.h>
#include <string>
#include <unistd.h>
#include "proto.h"

#define MAXLINE 1024
#define MAXIPLEN 16

int do_list(int sockfd, MSG *msg);
int do_getfile(int sockfd, MSG *msg, std::string targetFile);
int do_putfile(int sockfd, MSG *msg);

int main(int argc, char *argv[]) {
    int sockfd, n;
    char server_addr[MAXIPLEN] = "localhost";
    int server_port = 12345;
    std::string target_file;
    MSG msg;
    msg.type = DO_LIST;

    if (argc == 1) {
        msg.type = DO_LIST;
    } else {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            printf("%s\n%s\n\n%s\n%s\n\n%s\n%s\n%s\n%s\n%s\n",
            "Usage: fclient [OPTION] ...", "download file from fserver over tcp ", "options:",
            "    -l, --list                 show remote served files [default action]",
            "    -s, --server            specify server address [default addr is localhost]", 
            "    -p, --port                 specify server port [default port is 12345]",
            "    -d, --dl=FILE              download specified file",
            "    -o, --output=FILE        output to the FILE",
            "    -h, --help                display this help and exit");
            return 0;
        }

        for (int i = 1; i < argc; i+=2) {
            if (strcmp(argv[i], "-p") == 0) {
                server_port = atoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "-s") == 0) {
                memset(server_addr, 0, sizeof(server_addr));
                strcpy(server_addr, argv[i + 1]);
            } else if (strcmp(argv[i], "-l") == 0) {
                msg.type = DO_LIST;
            } else if (strcmp(argv[i], "-d") == 0) {
                strcpy(msg.buf, argv[i + 1]);
                msg.type = DO_GETFILE;
                target_file = argv[i + 1];
            } else if (strcmp(argv[i], "-o") == 0) {
                target_file = std::string(argv[i + 1]);
            }
        }
    }

    // printf("server:%s\nport:%d\n", server_addr, server_port);
    struct sockaddr_in servaddr;
    /* 创建套接字 */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }

    /* 构建结构体ip、port */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_addr, &servaddr.sin_addr.s_addr);

    /* 建立连接 */
    if (connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        char err[MAXLINE];
        sprintf(err, "failed to connect to %s:%d", server_addr, server_port);
        perror(err);
        exit(1);
    }
    
    switch (msg.type) {
        case 1:
            do_list(sockfd, &msg);
            break;
        case 2:
            do_getfile(sockfd, &msg, target_file);
            break;
        case 3:
            do_putfile(sockfd, &msg);
            break;
        default:
            close(sockfd);
            exit(0);
            break;
    }

    return 0;
}

int do_list(int sockfd, MSG* msg){
	send(sockfd,msg,sizeof(MSG),0);
	while(1){
		if(recv(sockfd,msg,sizeof(MSG),0) > 0){
            printf("%s",msg->buf);
		}
		if(msg->buf[0] == '\0'){
			// printf("List finish!!!\n");
			break;
		}
	}
	return 0;
}

int do_getfile(int sockfd, MSG *msg, std::string targetFile){
    printf("start downloading %s...\n", msg->buf);
	FILE *fp;
	fp = fopen(targetFile.c_str(), "w");
	send(sockfd, msg, sizeof(MSG), 0);
	while(1){
		recv(sockfd, msg, sizeof(MSG), 0);
		if(strncmp(msg->buf,"finished",8) == 0){
			printf("OK %s\n", targetFile.c_str());
			break;
		}
		fputs(msg->buf,fp);
	}
	fclose(fp);

	return 0;
}

int do_putfile(int sockfd, MSG *msg){

}