#include "client.h"
client_manager *client_manager::client_instance = new client_manager();
client_manager *client_manager::instance()
{
    // if (NULL == client_instance)
    // {
    // 	client_instance = new client_manager();
    // }
    return client_instance;
}
client_manager::client_manager()
{
    fundataCB=NULL;
    sock_info.socket_fd = -1;
    sock_info.is_connected = false;
    sock_info.connect_thread_id = -1;
    sock_info.recv_thread_id = -1;
    FD_ZERO(&sock_info.recv_set);
    pthread_mutex_init(&sock_info.thread_mutex, NULL);
    memset(&sock_info.servaddr, 0, sizeof(sock_info.servaddr));
    memset(sock_info.recv_buff, 0, MAXBUFFSIZE);
    sock_info.servaddr.sin_family = AF_INET;
    sock_info.servaddr.sin_port = htons(CONNECT_PORT);
    sock_info.servaddr.sin_addr.s_addr = inet_addr(CONNECT_ADDR);
}

int client_manager::client_init(RcvDataCallback funCB)
{
    if ((sock_info.socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf(" client :create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    if (run_connect() < 0)
    {
        printf(" client :run_connect error\n");
        return -1;
    }
    if (run_recv() < 0)
    {
        printf(" client :run_recv error\n");
        return -1;
    }
    if(funCB!=NULL)
    {
        fundataCB=funCB;
    }
    return 0;
}

int client_manager::client_send(void *buf, int size)
{
    int ret = send(sock_info.socket_fd, buf, size, MSG_NOSIGNAL);
    if (ret <= 0)
    {
        printf(" client: send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    //printf(" client: send msg success\n");
    return ret;
}

void *client_manager::recv_thread(void *arg)
{
    client_manager *thiz = (client_manager *)arg;
    struct timeval timeout;
    while (1)
    {
        if (thiz->get_connect_status())
        {
            FD_SET(thiz->sock_info.socket_fd, &thiz->sock_info.recv_set);
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            memset(thiz->sock_info.recv_buff, 0, MAXBUFFSIZE);
            thiz->sock_info.recv_size = -1;
            if (select(thiz->sock_info.socket_fd + 1, &thiz->sock_info.recv_set, NULL, NULL, &timeout) > 0)
            {
                if (FD_ISSET(thiz->sock_info.socket_fd, &thiz->sock_info.recv_set))
                {
                    thiz->sock_info.recv_size = recv(thiz->sock_info.socket_fd, thiz->sock_info.recv_buff, MAXBUFFSIZE, 0);
                    if (thiz->sock_info.recv_size <= 0)
                    {
                        if (thiz->sock_info.recv_size == 0)
                        {
                            if (errno == EINTR)
                                continue;
                            else
                            {
                                thiz->set_connect_stayus(false);
                                printf(" client:server exited,retry connecting... \n");
                            }
                        }
                        else
                        {
                            thiz->set_connect_stayus(false);
                            printf(" client:recv buf error,retry connect\n");
                        }
                    }
                    else
                    {
                        if(thiz->fundataCB!=NULL)
                        {
                            thiz->fundataCB(thiz->sock_info.recv_buff,thiz->sock_info.recv_size);
                        }
                        printf(" client: recv %d byte msg from server\n", (int)thiz->sock_info.recv_size);
                    }
                }
            }
        }
        else
        {
            sleep(2);
        }
    }
}

int client_manager::run_recv()
{
    if (pthread_create(&sock_info.recv_thread_id, NULL, recv_thread, this) > 0)
    {
        return -1;
    }
    return 0;
}

int client_manager::run_connect()
{
    if (pthread_create(&sock_info.connect_thread_id, NULL, connect_thread, this) > 0)
    {
        return -1;
    }
    return 0;
}

void *client_manager::connect_thread(void *arg)
{

    client_manager *thiz = (client_manager *)arg;
    while (1)
    {
        if (thiz->get_connect_status())
        {
            // if (thiz->client_send((void *)"heartbeat", 10) <= 0)
            // {
            //     thiz->set_connect_stayus(false);
            //     printf(" client:disconnected,retry connect server...\n");
            //     continue;
            // }
            sleep(2);
        }
        else
        {
            if (thiz->sock_info.socket_fd >= 0)
            {
                close(thiz->sock_info.socket_fd);
                thiz->sock_info.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            }
            if (connect(thiz->sock_info.socket_fd, (struct sockaddr *)&thiz->sock_info.servaddr, sizeof(thiz->sock_info.servaddr)) < 0)
            {
                thiz->set_connect_stayus(false);
                printf(" client :retry connect server(%s:%d)...\n", CONNECT_ADDR, CONNECT_PORT);
                sleep(2);
                continue;
            }
            else
            {
                thiz->set_connect_stayus(true);
                printf(" client :connect server success\n");
            }
        }
    }
}

void client_manager::client_close()
{
    if (sock_info.recv_thread_id >= 0)
    {
        if (pthread_cancel(sock_info.recv_thread_id) > 0)
            printf(" client:recv_thread cancel failed\n");
    }
    if (sock_info.connect_thread_id >= 0)
    {
        if (pthread_cancel(sock_info.connect_thread_id) > 0)
            printf(" client:connect_thread cancel failed\n");
    }
    if (sock_info.socket_fd >= 0)
    {
        if (close(sock_info.socket_fd) < 0)
            printf(" client:close socket_fd failed\n");
    }
    printf(" client: exit success\n");
}

bool client_manager::get_connect_status()
{
    bool status = false;
    pthread_mutex_lock(&sock_info.thread_mutex);
    status = sock_info.is_connected;
    pthread_mutex_unlock(&sock_info.thread_mutex);
    return status;
}
void client_manager::set_connect_stayus(bool value)
{
    pthread_mutex_lock(&sock_info.thread_mutex);
    sock_info.is_connected = value;
    pthread_mutex_unlock(&sock_info.thread_mutex);
}