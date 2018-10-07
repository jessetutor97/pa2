#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "packet.h"

int main(int argc, char *argv[])
{
    // Declare UDP socket
    int send_socket = 0;
    send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Get host IP address
    struct hostent *s;
    s = gethostbyname(argv[1]);

    // Set destination info
    struct sockaddr_in server;
    socklen_t s_len = sizeof(server);
    memset((char *)&server, 0, s_len);
    server.sin_family = AF_INET;
    uint16_t port1 = atoi(argv[2]);
    server.sin_port = htons(port1);
    bcopy((char *)s->h_addr, (char *)&server.sin_addr.s_addr, s->h_length);

    // Send data
    // TODO: rename a bunch of stuff
    char send_payload[5] = "test";
    packet mypacket(1, 0, 5, send_payload);
    char spacket[24];
    mypacket.serialize(spacket);
//    printf("%i\n", sizeof(mypacket));
    sendto(send_socket, spacket, 24, 0, (struct sockaddr *)&server, s_len);

    printf("Done\n");

    // Declare another UDP socket
    int receive_socket = 0;
    receive_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind socket to an address/port number
    bind(receive_socket, (struct sockaddr *)&server, s_len);

    char temparray[5];
    char receive_payload[24];
    packet dpacket(1, 0, 5, temparray);
    recvfrom(receive_socket, receive_payload, 24, 0, (struct sockaddr *)&server, &s_len);
    dpacket.deserialize(receive_payload);

    close(send_socket);
    close(receive_socket);
}
