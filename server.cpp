#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include "packet.h"

using namespace std;

int main(int argc, char *argv[]){

    // Get host IP address
    // struct hostent *s;
    // s = gethostbyname(argv[1]);

    // Set up port, declare variables
    int rcv_port = atoi(argv[2]);
    int bytes_recvd;
    char s_rcv_packet[24];

    // Declare UDP socket for receiving data
    int rcv_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Set destination info
    struct sockaddr_in rcv_server;
    socklen_t s_len = sizeof(rcv_server);
    memset((char *) &rcv_server, 0, s_len);
    rcv_server.sin_family = AF_INET;
    rcv_server.sin_port = htons(rcv_port);
    rcv_server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to an address/port number
    bind(rcv_socket, (struct sockaddr *)&rcv_server, s_len);

    // Wait for data to arrive
    bytes_recvd = recvfrom(rcv_socket, s_rcv_packet, 24, 0, (struct sockaddr *)&rcv_server, &s_len);

    // Deserialize packet and print
    char temparray[5] = {'0', '0', '0', '0', '0'};
    packet rcv_packet(1, 0, 5, temparray);
    rcv_packet.deserialize(s_rcv_packet);
    rcv_packet.printContents();

    // Declare UDP socket for sending an acknowledgement
    int send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Get host IP address
    // struct hostent *s;
    // s = gethostbyname(argv[1]);

    // TODO: change port numbers/create new sockaddr_in struct
    // Set destination info
    struct sockaddr_in send_server;
    memset((char *)&send_server, 0, s_len);
    send_server.sin_family = AF_INET;
    uint16_t send_port = atoi(argv[3]);
    /*
    send_server.sin_port = htons(send_port);
    bcopy((char *)s->h_addr, (char *)&send_server.sin_addr.s_addr, s->h_length);

    // Send acknowledge
    // char *b;
    packet send_packet(0, 0, 0, (char *)s);
    char s_send_packet[24];
    send_packet.serialize(s_send_packet);
    sendto(send_socket, s_send_packet, 24, 0, (struct sockaddr *)&send_server, s_len);
    */

    // Close sockets
    close(rcv_socket);
    // close(send_socket);
}
