CC = gcc
CFLAGS = -pedantic -Wextra -Wall -lpthread -g
all: udpserver udpclient

udpserver: udpserver.o
	${CC} ${CFLAGS} $< -o udpserver
udpserver.o: udpserver.c
	${CC} ${CFLAGS} -c $<
udpclient: udpclient.o
	${CC} ${CFLAGS} $< -o udpclient
udpclient.o: udpclient.c
	${CC} ${CFLAGS} -c $<
clean:
	rm udpserver udpclient *.o
