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
	int rcv_port = atoi(argv[2]);
	int bytes_recvd;
	char payload[24];

	int receive_socket = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in server;
	socklen_t s_len = sizeof(server);
	memset((char *) &server, 0, s_len);
	server.sin_family = AF_INET;
	server.sin_port = htons(rcv_port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to an address/port number
    bind(receive_socket, (struct sockaddr *)&server, s_len);

	bytes_recvd = recvfrom(receive_socket, payload, 24, 0, (struct sockaddr *)&server, &s_len);

    char temparray[5];
    packet mypacket(1, 0, 5, temparray);
    mypacket.deserialize(payload);
    mypacket.printContents();

    // Declare a UDP socket for sending an acknowledge
    int send_socket = 0;
    send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Get host IP address
    struct hostent *s;
    s = gethostbyname(argv[1]);

    // TODO: change port numbers/create new sockaddr_in struct

    // Send acknowledge
    packet spacket(0, 0, 0, temparray);
    printf("%i\n", sizeof(spacket));

    close(receive_socket);
}
