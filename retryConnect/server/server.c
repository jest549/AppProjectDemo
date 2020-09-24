#include "server.h"
socket_info sock_info;
int run_accept();
void* accept_thread(void *arg);
int server_init()
{ 
    sock_info.socket_fd=-1;
    sock_info.connect_fd=-1;  
    sock_info.accept_thread_id=-1; 
    memset(&sock_info.servaddr, 0, sizeof(sock_info.servaddr));  
    sock_info.servaddr.sin_family = AF_INET;  
    sock_info.servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_info.servaddr.sin_port = htons(LISTEN_PORT);
    if((sock_info.socket_fd = socket(AF_INET, SOCK_STREAM, 0))<0)
    {  
        printf("server: create  error: %s(errno: %d)\n",strerror(errno),errno); 
        return -1; 
    }
    int bReuseaddr = 1;
	if (setsockopt(sock_info.socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseaddr, sizeof(int)) < 0)
	{
		printf("set socket option error !");
		return -1;
	}
    if(bind(sock_info.socket_fd, (struct sockaddr*)&sock_info.servaddr, sizeof(sock_info.servaddr))<0)
    {  
        printf("server: bind  error: %s(errno: %d)\n",strerror(errno),errno);
        return -1;  
    }   

    if(listen(sock_info.socket_fd, MAXCONNECTNUM) <0)
    {  
        printf("server: listen  error: %s(errno: %d)\n",strerror(errno),errno);
        return -1;  
    } 
    if(run_accept()<0) 
    {
        printf("server: run_accept  error\n");
        return -1;
    }
    printf("server: init success and waiting for  client connect\n"); 
    return 0; 
}

int run_accept()
{
	
	if(pthread_create(&sock_info.accept_thread_id, NULL, accept_thread, NULL)>0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void* accept_thread(void *arg)
{
    while(1)
    {    
        if( (sock_info.connect_fd = accept(sock_info.socket_fd, (struct sockaddr*)NULL, NULL)) <0)
        {  
            printf("server: accept  error: %s(errno: %d)\n",strerror(errno),errno);  
            continue;  
        }
        else
        {       
            printf("server: client connect success\n");
            while(1)
            {
                memset(sock_info.recv_buff,0,MAXLINE); 
                sock_info.recv_size=-1;
                if(sock_info.connect_fd!=-1) 
                {
                    if((sock_info.recv_size= recv(sock_info.connect_fd, sock_info.recv_buff, MAXLINE, 0))<=0)
                    {   
                        if(errno == EINTR)
                        {
                            continue;
                        }
                        else
                        {
                            printf("server: client exited,waiting client connect...\n");
                            close(sock_info.connect_fd);
                            sock_info.connect_fd=-1;
                            break;
                        }
                        
                    } 
                    else
                    {
                        printf("server:recv %d byte msg from client: %s\n", (int)sock_info.recv_size,(char*)sock_info.recv_buff); 
                        send(sock_info.connect_fd, (char*)sock_info.recv_buff, sock_info.recv_size,MSG_NOSIGNAL);
                    }
                }
            }
        }
    }  
}
void server_close()
{   
    if(sock_info.accept_thread_id>=0)
    {   
        if(pthread_cancel(sock_info.accept_thread_id)>0)
        printf("server:server exit failed(pthread_cancel)\n");
    }
    if(sock_info.connect_fd>=0)
    {
        if( close(sock_info.connect_fd)<0)
            printf("server:server exit failed(close fd)\n");
    }
    if(sock_info.socket_fd>=0)
    {
        if(close(sock_info.socket_fd)<0)
            printf("server:server exit failed(close fd)\n");
    }   
    printf("server:server exit success\n");
}
 
