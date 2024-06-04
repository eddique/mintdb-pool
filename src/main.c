#include <stdio.h>
#include "server.h"

int main(int argc, char* argv[])
{
    int port = 4321;
    printf("\n\x1b[38;5;50mmintdb\x1b[0mpool\n\n");
    server(port);
    return 0;
}