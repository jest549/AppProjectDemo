#include "server.h"
int main(int argc, char* argv[])
{
    server_init();
    while(1)
    {
        sleep(10);
    }
}