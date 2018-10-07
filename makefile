all: client server

client: client.o
	g++ -Wall client.cpp packet.cpp -o client
	
server: server.o
	g++ -Wall server.cpp packet.cpp -o server	

clean:
	\rm *.o client server
