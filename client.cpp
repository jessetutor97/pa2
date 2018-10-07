#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Declare UDP socket
    int socket1 = 0;
    socket1 = socket(AF_INET, SOCK_DGRAM, 0);
}
