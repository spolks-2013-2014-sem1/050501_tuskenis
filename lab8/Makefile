CC=gcc
CP=g++
SOURCES_CLIENT=client.c
SOURCES_SERVER=main.cpp server_tcp.cpp server_udp.cpp
LIBS=../lib-spolks/utils.c ../lib-spolks/sockets_wrap.c
LIBS_CPP=../lib-spolks/semaphore_wrap.cpp
EXECUTABLE_CLIENT=client
EXECUTABLE_SERVER=server

all:

	$(CC) $(SOURCES_CLIENT) $(LIBS) -o $(EXECUTABLE_CLIENT)
	$(CP) $(SOURCES_SERVER) $(LIBS_CPP) $(LIBS) -o $(EXECUTABLE_SERVER)
