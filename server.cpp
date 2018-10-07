#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]){
	int rcv_port = 4578;
	int bytes_recvd;
	char payload[5];

	int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in server;
	socklen_t s_len = sizeof(server);
	memset((char *) &server, 0, s_len);
	server.sin_family = AF_INET;
	server.sin_port = htons(rcv_port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind docket to an address/port number
    bind(udp_socket, (struct sockaddr *)&server, s_len);

	bytes_recvd = recvfrom(udp_socket, payload, 5, 0, (struct sockaddr *)&server, &s_len);
	printf("%s\n", payload);



}
