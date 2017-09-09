/*
 * udpserver.h - заголовочный файл многопоточного UDP сервера с очередью
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/msg.h>
#define BACKLOG 10
#define BUFLEN 256
#define THREADS_NUM 1
enum states{LISTENING, REQUEST, NOPE, TOQUEUE};

struct sockreq {
    long      type;
    char      msg[BUFLEN];
    socklen_t addr_len;

    struct sockaddr_storage their_addr;
};

struct tothread{
    int sockfd;
    int msqid;
};
static pthread_mutex_t thrdmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid[THREADS_NUM];

char state;
int create_msqid(char *progname, int pid);
int create_socket(char *port);
void *process_rquest();
int check_request(char *msg);
void toqueue(int msqid, struct sockreq rquest);
struct sockreq listening(int sockfd);
void nope();
