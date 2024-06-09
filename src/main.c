#include <stdio.h>
#include "server.h"

int main(int argc, char* argv[])
{
    printf("\n\x1b[38;5;50mmintdb\x1b[0mpool\n\n");
    server();
    return 0;
}