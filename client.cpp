#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "packet.h"

int main(int argc, char *argv[]) {
    // Declare UDP socket for sending data
    int send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Get host IP address
    struct hostent *s;
    s = gethostbyname(argv[1]);

    // Set destination info
    struct sockaddr_in send_server;
    socklen_t s_len = sizeof(send_server);
    memset((char *)&send_server, 0, s_len);
    send_server.sin_family = AF_INET;
    int send_port = atoi(argv[2]);
    send_server.sin_port = htons(send_port);
    bcopy((char *)s->h_addr, (char *)&send_server.sin_addr.s_addr, s->h_length);

    // Send data
    char send_payload[5] = "test";
    packet send_packet(1, 0, 5, send_payload);
    char s_send_packet[24];
    send_packet.serialize(s_send_packet);
    sendto(send_socket, s_send_packet, 24, 0, (struct sockaddr *)&send_server, s_len);

    // Declare UDP socket for receiving acknowledgement
    int rcv_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Set destination info
    struct sockaddr_in rcv_server;
    memset((char *)&rcv_server, 0, s_len);
    rcv_server.sin_family = AF_INET;
    int rcv_port = atoi(argv[3]);
    rcv_server.sin_port = htons(rcv_port);
    rcv_server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to an address/port number
    bind(rcv_socket, (struct sockaddr *)&rcv_server, s_len);

    // Wait for data to arrive
    char s_rcv_packet[24];
    recvfrom(rcv_socket, s_rcv_packet, 24, 0, (struct sockaddr *)&rcv_server, &s_len);

    // Deserialize packet and print
    packet rcv_packet(4, 0, 0, 0);
    rcv_packet.deserialize(s_rcv_packet);
    rcv_packet.printContents();

    // Close sockets
    close(send_socket);
    close(rcv_socket);
}
