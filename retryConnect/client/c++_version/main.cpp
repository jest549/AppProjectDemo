#include "client.h"
int main(int argc, char* argv[])
{
    client_manager::instance()->client_init(NULL);
    while(1)
    {
        sleep(10);
    }
}