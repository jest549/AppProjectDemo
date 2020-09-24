#ifndef CLIENT_TO_UA_H
#define CLIENT_TO_UA_H
#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<errno.h>
#include<unistd.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h> 
#include<arpa/inet.h> 
#include<pthread.h>
#include <sys/select.h>
#include<stdbool.h>
#define CONNECT_PORT (8888)
#define CONNECT_ADDR "127.0.0.1"   
#define MAXBUFFSIZE (2048) 
 
typedef struct socket_infomation
{
    int     socket_fd;
    bool     is_connected;  
    char    recv_buff[MAXBUFFSIZE]; 
    int  recv_size;
    fd_set  recv_set;
    struct  sockaddr_in servaddr;
    pthread_t  connect_thread_id; 
    pthread_t  recv_thread_id;  
    pthread_mutex_t thread_mutex;
}socket_info;
typedef void (*RcvDataCallback)(char* buff,int size);
class client_manager
{
    private:
        static client_manager * client_instance;
        static void* recv_thread(void *arg);
        static void* connect_thread(void *arg);
        client_manager();
        socket_info sock_info;
        int  run_recv();
        int  run_connect();
        bool get_connect_status();
        void set_connect_stayus(bool value);
        RcvDataCallback fundataCB;
    public:
        static client_manager * instance();
        int  client_init(RcvDataCallback func);
        int  client_send(void *buf,int size);
        void  client_close();

};


#endif