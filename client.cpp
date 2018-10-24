#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include "packet.h"

int ack_calc(int seq_num, int send_base) {
    int temp_num = (send_base + 7) % 8;
    int count = 0;
    while (temp_num != seq_num) {
        temp_num = (temp_num + 1) % 8;
        ++count;
    }
    return count;
}

int main(int argc, char *argv[]) {
    // Declare UDP socket for sending data
    int send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Get host IP address
    struct hostent *s;
    s = gethostbyname(argv[1]);

    // Print host name and IP address
    printf("-----------------------------------------\n\n");
    printf("Host name: %s\n", argv[1]);
    printf("IP address: %i.%i.%i.%i\n\n", s->h_addr[0], s->h_addr[1], s->h_addr[2], s->h_addr[3]);
    printf("-----------------------------------------\n");

    // Set destination info
    struct sockaddr_in send_server;
    socklen_t s_len = sizeof(send_server);
    memset((char *)&send_server, 0, s_len);
    send_server.sin_family = AF_INET;
    int send_port = atoi(argv[2]);
    send_server.sin_port = htons(send_port);
    bcopy((char *)s->h_addr, (char *)&send_server.sin_addr.s_addr, s->h_length);

    // Declare UDP socket for receiving acknowledgements
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

    // Open file for reading
    FILE *fp;
    fp = fopen(argv[4], "r");
    FILE *seq_num_file;
    seq_num_file = fopen("seqnum.log", "w");
    FILE *ack_file;
    ack_file = fopen("ack.log", "w");

    // Determine number of bytes in file
    int num_bytes = 0;
    char data = 0;
    while (data != EOF) {
        data = fgetc(fp);
        ++num_bytes;
    }
    --num_bytes;
    rewind(fp);

    // Determine the number of packets to be sent
    int num_full_pckts = num_bytes / 30;
    int num_pckts = num_full_pckts;
    int num_final_bytes = num_bytes % 30;
    if (num_final_bytes != 0) {
        ++num_pckts;
    }

    // Create the full packets and store in an array
    packet* pckt_array[num_pckts];
    char* payload_array[num_pckts];
    char* s_pckt_array[num_pckts];
    int seq_num = 0;
    for (int i = 0; i < num_full_pckts; ++i) {
        payload_array[i] = new char[30];
        for (int j = 0; j < 30; ++j) {
            payload_array[i][j] = fgetc(fp);
        }
        pckt_array[i] = new packet(1, seq_num++, 30, payload_array[i]);
        s_pckt_array[i] = new char[37];
        pckt_array[i]->serialize(s_pckt_array[i]);
        seq_num %= 8;
    }

    // Add the last packet if necessary
    if (num_final_bytes != 0) {
        payload_array[num_pckts - 1] = new char[num_final_bytes];
        for (int i = 0; i < num_final_bytes; ++i) {
            payload_array[num_pckts - 1][i] = fgetc(fp);
        }
        pckt_array[num_pckts - 1] = new packet(1, seq_num++, num_final_bytes, payload_array[num_pckts - 1]);
        s_pckt_array[num_pckts - 1] = new char[37];
        pckt_array[num_pckts - 1]->serialize(s_pckt_array[num_pckts - 1]);
        seq_num %= 8;
    }

    // Start transmission
    int w_size = 7;
    if (num_pckts < 7) {
        w_size = num_pckts;
    }
    struct pollfd ufds;
    ufds.fd = rcv_socket;
    ufds.events = POLLIN;
    int send_base = 0;
    int next_sn = 0;
    int o_pckts = 0;
    int next_pckt = 0;
    int pckt_ack_count = 0;
    int timeout = 0;
    packet rcv_packet(4, 0, 0, 0);
    char s_rcv_packet[24];
    bool transmitting = true;
    while (transmitting) {
        // Check if the window is full
        while (next_pckt - send_base < w_size && next_pckt < num_pckts) {
            // If window is not full, send a packet and update variables
            sendto(send_socket, s_pckt_array[next_pckt], 37, 0, (struct sockaddr *)&send_server, s_len);
            // Write sequence numbers of sent packets to log file
            fprintf(seq_num_file, "%i\n", pckt_array[next_pckt]->getSeqNum());
            pckt_array[next_pckt++]->printContents();
            next_sn = (next_sn + 1) % 8;
            ++o_pckts;
            printf("SB: %i\n", send_base);
            printf("NS: %i\n", next_sn);
            printf("Number of outstanding packets: %i\n", o_pckts);
            printf("Window size: %i\n", w_size);
            printf("--------------------------------------\n");
        }
        // Wait for acknowledge
        timeout = poll(&ufds, 1, 2000);
        if (timeout > 0) {
            recvfrom(rcv_socket, s_rcv_packet, 24, 0, (struct sockaddr *)&rcv_server, &s_len);
        }

        // If alarm interrupts receive call, retransmit all outstanding packets
        else if (timeout == 0) {
            printf("Timeout occurred, resending packets\n");
            next_pckt = send_base;
            next_sn = send_base % 8;
            o_pckts = 0;
            continue;
        }

        // Deserialize packet and print
        rcv_packet.deserialize(s_rcv_packet);
        rcv_packet.printContents();

        // Write sequence number of received packets to log file
        fprintf(ack_file, "%i\n", rcv_packet.getSeqNum()); 

        // If packet is lost, retransmit
        if (rcv_packet.getSeqNum() == (send_base + 7) % 8){
            printf("Packet was lost, resending packets\n");
            next_pckt = send_base;
            next_sn = send_base % 8;
            o_pckts = 0;
            continue;
        }

        // Update variables
        pckt_ack_count = ack_calc(rcv_packet.getSeqNum(), send_base);
        send_base += pckt_ack_count;
        if (next_pckt == num_pckts) {
            w_size -= pckt_ack_count;
        }
        o_pckts -= pckt_ack_count;
        printf("SB: %i\n", send_base % 8);
        printf("NS: %i\n", next_sn);
        printf("Number of outstanding packets: %i\n", o_pckts);
        printf("Window size: %i\n", w_size);
        printf("--------------------------------------\n");
        if (w_size == 0) {
            transmitting = false;
        }
    }

    // Send EOT packet
    char s_eot_packet[24];
    packet eot_packet(3, next_sn, 0, 0);
    // Write sequence number of sent EOT packet to log file
    fprintf(seq_num_file, "%i\n", eot_packet.getSeqNum());
    eot_packet.serialize(s_eot_packet);
    printf("Sending EOT packet to server.\n");
    sendto(send_socket, s_eot_packet, 24, 0, (struct sockaddr *)&send_server, s_len);
    eot_packet.printContents();

    // Wait for EOT from server
    while (eot_packet.getType() != 2) {
        recvfrom(rcv_socket, s_eot_packet, 24, 0, (struct sockaddr *)&rcv_server, &s_len);
        eot_packet.deserialize(s_eot_packet);
        eot_packet.printContents();
        // Write sequence number of received EOT packet to log file
        fprintf(ack_file, "%i\n", eot_packet.getSeqNum());
    }
    printf("EOT packet received. Exiting.--------------------------------------\n");

    // Delete array data
    for (int i = 0; i < num_pckts; ++i) {
        delete pckt_array[i];
        delete payload_array[i];
        delete s_pckt_array[i];
    }

    // Close files
    fclose(fp);
    fclose(ack_file);
    fclose(seq_num_file);

    // Close sockets
    close(send_socket);
    close(rcv_socket);
}
