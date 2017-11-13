//
//  tcp_server.cpp
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

#define DEFAULT_PORT 8888
#define MAXLINE 1024
#define LISTENQ 10

int tcp_listen(int port);
void accept_cb(int fd, short events, void *arg);
void socket_read_cb(int fd, short events, void *arg);

int main(int argc, char **argv)
{
    int port = DEFAULT_PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    printf("http port: %d\n", port);
    
    int listenfd;
    if ( (listenfd = tcp_listen(port)) < 0) {
        perror("tcp_listen error");
        return 1;
    }
    
    struct event_base *base = event_base_new();
    
    struct event *ev_listen = event_new(base, listenfd, EV_READ | EV_PERSIST, accept_cb, base);
    event_add(ev_listen, NULL);
    
    event_base_dispatch(base);
    
    return 0;
}

void accept_cb(int fd, short events, void *arg)
{
    int sockfd = accept(fd, (struct sockaddr*)NULL, NULL);
    printf("client fd: %d\n", sockfd);
    
    struct event_base *base = (struct event_base*)arg;
    struct event *ev = (struct event*)malloc(sizeof(struct event));
    event_assign(ev, base, sockfd, EV_READ | EV_PERSIST, socket_read_cb, ev);
    event_add(ev, NULL);
}

void socket_read_cb(int fd, short events, void *arg)
{
    char msg[MAXLINE];
    int len = read(fd, msg, MAXLINE);
    if (len > 0) {
        msg[len] = '\0';
        printf("msg from client %d: %s\n", fd, msg);
        char reply_msg[MAXLINE] = "server has got your msg";
        write(fd, reply_msg, strlen(reply_msg));
    } else {
        if (len == 0) {
            printf("client %d closed\n", fd);
        } else {
            perror("read error");
        }
        struct event *ev = (struct event*)arg;
        event_free(ev);
        close(fd);
    }
}

int tcp_listen(int port)
{
    int listenfd;
    if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket error");
        return -1;
    }
    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    
    if ( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("socket error");
        return -1;
    }
    
    if ( listen(listenfd, LISTENQ) < 0 ) {
        perror("listen error");
        return -1;
    }
    return listenfd;
}
