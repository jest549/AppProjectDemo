#ifndef SERVER_FOR_OTA_H
#define SERVER_FOR_OTA_H
#include<stdio.h>  
#include<stdbool.h>
#include<stdlib.h>  
#include<string.h>  
#include<errno.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h> 
#include<unistd.h> 
#include<pthread.h>
#define LISTEN_PORT 8888 
#define MAXLINE 2048 
#define MAXCONNECTNUM 5

typedef struct socket_infomation
{
    int        socket_fd,connect_fd;  
    struct     sockaddr_in servaddr;  
    char       recv_buff[MAXLINE];  
    int     recv_size;
    pthread_t  accept_thread_id; 
}socket_info;
int   server_init();
void  server_close();
#endif