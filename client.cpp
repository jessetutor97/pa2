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

    // Get host IP address
    struct hostent *s;
    s = gethostbyname("localhost");

    // Set destination info
    struct sockaddr_in server;
    socklen_t server_size = sizeof(server);
    memset((char *)&server, 0, server_size);
    server.sin_family = AF_INET;
    uint16_t port1 = 4578;
    server.sin_port = htons(port1);
    bcopy((char *)s->h_addr, (char *)&server.sin_addr.s_addr, s->h_length);

    // Send data
    char payload[5] = "test";
    sendto(socket1, payload, 5, 0, (struct sockaddr *)&server, server_size);

    printf("Done\n");

    close(socket1);
}
