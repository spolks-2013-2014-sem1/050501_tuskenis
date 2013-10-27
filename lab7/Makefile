CC=gcc
CP=g++
SOURCES_CLIENT=client.c
SOURCES_SERVER=main.cpp server_tcp.cpp server_udp.cpp
LIBS=../lib-spolks/utils.c ../lib-spolks/sockets_wrap.c
EXECUTABLE_CLIENT=client
EXECUTABLE_SERVER=server

all:

	$(CC) $(SOURCES_CLIENT) $(LIBS) -o $(EXECUTABLE_CLIENT) -pthread
	$(CP) $(SOURCES_SERVER) $(LIBS) -o $(EXECUTABLE_SERVER) -pthread
