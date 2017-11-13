//
//  tcp_client.cpp
//  libevent_debug
//
//  Created by weijiazhen on 17/10/23.
//  Copyright © 2017年 weijiazhen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <event2/event.h>
#include <event2/event_struct.h>

#define MAXLINE 1024
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8888

int tcp_connect(const char *host, int port);
void cmd_msg_cb(int fd, short events, void *arg);
void socket_read_cb(int fd, short events, void *arg);

int main(int argc, char **argv)
{
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    } else if (argc == 3){
        host = argv[1];
        port = atoi(argv[2]);
    }
    printf("host: %s, port: %d\n", host, port);
    
    int sockfd;
    if ( (sockfd = tcp_connect(host, port)) < 0 ) {
        perror("tcp_connect error");
        return 1;
    }
    printf("Connect to the server successfully\n");
    
    struct event_base *base = event_base_new();
    
    struct event *ev_sockfd = event_new(base, sockfd, EV_READ | EV_PERSIST, socket_read_cb, NULL);
    event_add(ev_sockfd, NULL);
    
    struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_msg_cb, (void*) &sockfd);
    event_add(ev_cmd, NULL);
    
    event_base_dispatch(base);
    
    return 0;
}

void cmd_msg_cb(int fd, short events, void *arg)
{
    char msg[MAXLINE];
    int len = read(fd, msg, MAXLINE);
    if (len <= 0) {
        perror("read error");
        exit(1);
    }
    
    int sockfd = *((int*)arg);
    // 把终端的输入发送到服务器
    write(sockfd, msg, len);
}

void socket_read_cb(int fd, short events, void *arg)
{
    char msg[MAXLINE];
    int len = read(fd, msg, MAXLINE);
    if (len <= 0) {
        perror("read error");
        exit(1);
    }
    msg[len] = '\0';
    printf("msg from server: %s\n", msg);
}

int tcp_connect(const char *host, int port)
{
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }
    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", host);
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }
    
    return sockfd;
}
