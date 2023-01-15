#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <wait.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <string.h>
#include "process.h"
#include "proto.h"

#define MAXIPLEN 16
#define MAXLINE 1024
#define DEFAULT_FILEPATH "/home/root/cppProject/lab/lab3/file"

std::vector<std::string> showFileList(char *path);
bool checkValidPort(int port);
bool checkValidIp(char *ipStr);
int do_list(int acceptfd, MSG *msg);
int do_getfile(int acceptfd, MSG *msg);
int do_putfile(int acceptfd, MSG *msg);
int do_client(int acceptfd);

int main(int argc, char *argv[])
{
    int lfd, cfd, i;
    pid_t pid;
    socklen_t clen;
    struct sockaddr_in s_addr, c_addr;
    int port = 12345;
    char addr[MAXIPLEN] = "localhost";

    if (argc <= 1) {
        exit(1);
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        printf("%s\n%s\n\n%s\n%s\n\n%s\n%s\n%s\n%s\n%s\n",
        "Usage: fserver [OPTION] ... <DIR>", "serve some files over tcp", "args:", "    <DIR> Which directory to serve",
        "options:", "    -l, --listen=ADDR       specify source address to use [default is localhost]",
        "    -H, --hidden             show hidden file",
        "    -p, --port=PORT         specify listening port [default port is 12345]",
        "    -h, --help                display this help and exit");
        return 0;
    }

    for (i = 1; i < argc; i+=2) {
        if (strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
            if (!checkValidPort(port)) {
                printf("failed to start fserver: %d is not a valid port\n", port);
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-l") == 0) {
            memset(addr, 0, sizeof(addr));
            strcpy(addr, argv[i + 1]);
            if (!checkValidIp(addr)) {
                printf("failed to start fserver: %s is not a valid address\n", addr);
                exit(1);
            }
        }
    }
    // printf("host:%s\nport:%d\n", addr, port);

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket()");
        exit(1);
    }
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    inet_pton(AF_INET, addr, &s_addr.sin_addr.s_addr);

    if (bind(lfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0) {
        printf("failed to start fserver: %s:%d already in use\n", addr, port);
        exit(1);
    }
    if (listen(lfd, 200) < 0) {
        perror("listen()");
        exit(1);
    } else {
        printf("server listening at %s:%d\n", addr, port);
        printf("file lists:\n");
        std::vector<std::string> fileNameList = showFileList(DEFAULT_FILEPATH);
        for (int i = 0; i < fileNameList.size(); i++) {
            std::cout<<"    "<<fileNameList[i]<<std::endl;
        }
    }


    while(1)
    {
        clen = sizeof(c_addr);
        cfd = accept(lfd, (struct sockaddr *)&c_addr, &clen);
        if ((pid = fork()) == 0) //child
        {
            close(lfd);
            do_client(cfd);
            exit(0);
        } else if(pid > 0) {     //parent
            close(cfd);
        }
    }
    return 0;
}

//打印指定目录文件列表
std::vector<std::string> showFileList(char *path)
{
    std::vector<std::string> res;
    char *p;
    const char *d = "\n";
    chdir(path);
    Process process;
    std::vector<char*> command_v;
    command_v.push_back(const_cast<char*>("/usr/bin/ls"));
    command_v.push_back(const_cast<char*>(path));
    char **command = &command_v[0];
    int ret = process.exec(command[0], command, environ);
    std::string output = process.getExecOutput();
    //std::cout<<output;
    p = strtok(const_cast<char*> (output.c_str()), d);
    while (p)
    {
        struct stat file_stat;
        stat(p, &file_stat);
        if (S_ISREG(file_stat.st_mode)) {
            res.push_back(p);
        }
        p = strtok(NULL, d);
    }
    return res;
}

bool checkValidPort(int port) {
    if (port <= 0 || port >= 65535) {
        return false;
    } 
    // else {
    //     std::string p = std::to_string(port);
    //     Process process;
    //     std::vector<char*> command_v;
    //     command_v.push_back(const_cast<char*>("/usr/bin/env"));
    //     command_v.push_back(const_cast<char*>("netstat"));
    //     command_v.push_back(const_cast<char*>("-anp"));
    //     command_v.push_back(const_cast<char*>("|"));
    //     command_v.push_back(const_cast<char*>("/usr/bin/grep"));
    //     command_v.push_back(const_cast<char*>(p.c_str()));
    //     char **command = &command_v[0];
    //     int ret = process.exec(command[0], command, environ, 0);
    //     std::string output = process.getExecOutput();
    //     std::cout<<output<<std::endl;
    //     if (output != "")
    //         return false;
    //     else
    //         return true;
    // }
    return true;
}

bool checkValidIp(char *ipStr) {
    if (strcmp(ipStr, "localhost") == 0) {
        return true;
    }
    int count = 0;
    char *p, *strcp;
    const char *d = ".";
    strcpy(strcp, ipStr);
    p = strtok(strcp, d);
    while(p) {
        int num = atoi(p);
        if (num < 0 || num > 255) {
            return false;
        }
        count++;
        p = strtok(NULL, d);
    }
    if (count != 4) {
        return false;
    }
    return true;
}

int do_client(int acceptfd) {
    MSG msg;
    while (recv(acceptfd, &msg, sizeof(msg), 0) > 0) {
        switch (msg.type) {
            case 1:
                do_list(acceptfd, &msg);
                break;
            case 2:
                do_getfile(acceptfd, &msg);
                break;
            case 3:
                do_putfile(acceptfd, &msg);
            default:
                printf("invalid data msg\n");
                break;
        }
    }
    close(acceptfd);
    exit(0);
    return 0;
}

int do_list(int acceptfd, MSG *msg) {
    chdir("/home/root/cppProject/lab");
    FILE *fp;
    char cmd[MAXLINE];
    strcat(cmd, "ls ");
    strcat(cmd, DEFAULT_FILEPATH);
    strcat(cmd, " >temp");
    system(cmd);
    if ((fp = fopen("temp", "r")) == NULL) {
        perror("fopen failed");
        exit(0);
    }
    while (fgets(msg->buf, sizeof(MSG), fp) != NULL) {
        send(acceptfd, msg, sizeof(MSG), 0);
        // printf("msg: %s\n", msg->buf);
    }
    fclose(fp);
    msg->buf[0] = '\0';
    send(acceptfd, msg, sizeof(MSG), 0);
    system("rm temp");
    return 0;
}

int do_getfile(int acceptfd, MSG *msg) {
    FILE *fp;
    if ((fp = fopen(msg->buf, "r")) == NULL) {
        perror("fopen()");
        exit(1);
    }
    while (fgets(msg->buf, sizeof(MSG), fp) != NULL) {
        send(acceptfd, msg, sizeof(MSG), 0);
    }
    fclose(fp);
    strcpy(msg->buf, "finished");
    send(acceptfd, msg, sizeof(MSG), 0);
    return 0;
}

int do_putfile(int acceptfd, MSG *msg) {
    FILE *fp;
    fp = fopen(msg->buf, "w+");
    while (1) {
        recv(acceptfd, msg, sizeof(MSG), 0);
        if (msg->buf[0] == '\0')
            break;
        fputs(msg->buf, fp);
    }
    fclose(fp);
    return 0;
}