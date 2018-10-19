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

int main(int argc, char *argv[]) {
    // Set up port, declare variables
    int rcv_port = atoi(argv[2]);
    int bytes_recvd;
    char s_rcv_packet[37];

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

    // Declare UDP socket for sending an acknowledgement
    int send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Get host IP address
    struct hostent *s;
    s = gethostbyname(argv[1]);

    // Set destination info
    struct sockaddr_in send_server;
    memset((char *)&send_server, 0, s_len);
    send_server.sin_family = AF_INET;
    int send_port = atoi(argv[3]);
    send_server.sin_port = htons(send_port);
    bcopy((char *)s->h_addr, (char *)&send_server.sin_addr.s_addr, s->h_length);

    // Open file for writing
    FILE *fp;
    fp = fopen(argv[4], "w");

    // Wait for data to arrive
    int sn_expecting = 0;
    int seq_num = 0;
    char rcv_payload[30] = {0};
    char s_ack_packet[24];
    packet rcv_packet(1, 0, 30, rcv_payload);
    bool first_packet = true;
    bool receiving = true;
    while (receiving) {
        recvfrom(rcv_socket, s_rcv_packet, 37, 0, (struct sockaddr *)&rcv_server, &s_len);
        rcv_packet.deserialize(s_rcv_packet);
        printf("-----------------------------------------\n");
        rcv_packet.printContents();
        printf("Expecting Sn: %i\n", sn_expecting);
        printf("sn: %i\n", rcv_packet.getSeqNum());
        if (rcv_packet.getSeqNum() != sn_expecting && first_packet) {
            continue;
        }
        if (rcv_packet.getSeqNum() != sn_expecting) {
            packet ack_packet(0, seq_num, 0, 0);
            ack_packet.serialize(s_ack_packet);
            sendto(send_socket, s_ack_packet, 24, 0, (struct sockaddr *)&send_server, s_len);
            ack_packet.printContents();
            continue;
        }

        first_packet = false;
        seq_num = rcv_packet.getSeqNum();
        sn_expecting = (sn_expecting + 1) % 8;

        // Stop expecting packets if an EOT packet is received
        if (rcv_packet.getType() == 3) {
            char s_eot_packet[24];
            packet eot_packet(2, seq_num, 0, 0);
            eot_packet.serialize(s_eot_packet);
            sendto(send_socket, s_eot_packet, 24, 0, (struct sockaddr *)&send_server, s_len);
            eot_packet.printContents();
            receiving = false;
            continue;
        }

        // Send acknowledge back
        packet ack_packet(0, seq_num, 0, 0);
        ack_packet.serialize(s_ack_packet);
        sendto(send_socket, s_ack_packet, 24, 0, (struct sockaddr *)&send_server, s_len);
        ack_packet.printContents();

        // Write data to file
        for (int i = 0; i < 30; ++i) {
            if (rcv_payload[i] != 0) {
                fputc(rcv_payload[i], fp);
            }
        }
        memset(rcv_payload, 0, 30);
    }
    printf("-----------------------------------------\n");

    // Close file
    fclose(fp);

    // Close sockets
    close(rcv_socket);
    close(send_socket);
}
