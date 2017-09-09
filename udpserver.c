/*
 * многопоточный udp сервер 
 */
#include "udpserver.h"
void *process_rquest(void *arg)
{
    struct sockreq rquest;
    struct tothread poszel;
    int size;
    int gotmsg;
    size = sizeof(rquest.msg) +
           sizeof(rquest.addr_len) +
           sizeof(rquest.their_addr);

    printf("вошел в поток\n");
    poszel = *((struct tothread *) arg);
    while (1) {
        pthread_mutex_lock(&thrdmutex);
        gotmsg = msgrcv(poszel.msqid, &rquest, size, 0, IPC_NOWAIT);
        if (gotmsg != -1){
            /* смог вытащить сообщеие из очереди */
            pthread_mutex_unlock(&thrdmutex);

            printf("обрабатываю...\n");
            /* TODO: должно быть в цикле */
            sendto(poszel.sockfd, "АЫ", strlen("АЫ"), 0,
            (struct sockaddr *) &rquest.their_addr,  rquest.addr_len);

            printf("%s\n", rquest.msg);

            /* TODO: написать обработку какого запроса окончил */
            printf("Закончил обработку запроса\n");
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd;
    int msqid;
    int tmp;
    int i;
    struct sockreq rquest;
    struct tothread poszel;
    /* нам нужно знать порт  */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        return 1;
    }
    /* на этот сокет будем все слушать */
    sockfd = create_socket(argv[1]);

    /* создаем новую очередь */
    msqid = create_msqid(argv[0], getpid());
    pthread_mutex_lock(&thrdmutex);

    poszel.sockfd = sockfd;
    poszel.msqid = msqid;
    for (i = 0; i < THREADS_NUM; i++) {
        pthread_create(&tid[i], NULL, process_rquest, &poszel);
    }
    state = LISTENING;
    while(1) {
        switch(state) {
            case LISTENING:
                rquest = listening(sockfd);
                break;
            case REQUEST:
                tmp = check_request(rquest.msg);
                if(tmp)
                    state = TOQUEUE;
                else
                    state = NOPE;
                break;
            case NOPE:
                nope();
                state = LISTENING;
                break;
            case TOQUEUE:
                toqueue(msqid, rquest);
                pthread_mutex_unlock(&thrdmutex);
                state = LISTENING;
                break;
            default:
                nope();
                break;
        }
    }
    close(sockfd);
    return 0;
}

void nope()
{
    /* TODO: пусть еще адрес пишет */
    printf("Не могу обработать запрос");

}
int check_request(char *msg)
{
    if(strlen(msg) != 0 || strlen(msg) < BUFLEN)
        return 1;
    return 0;
}

void toqueue(int msqid, struct sockreq rquest)
{
    struct sockreq tmp;
    int size;

    size = sizeof(tmp.msg) + sizeof(tmp.addr_len) + sizeof(tmp.their_addr);
    tmp = rquest;


    /* TODO: чекнуть, что добавилось в очередь */
    msgsnd(msqid, (void *) &tmp, size, IPC_NOWAIT);

}

int create_msqid(char *progname, int pid)
{
    int   msqid;
    key_t key;
    /* сделай новый ключ для очереди */
    if((key = ftok(progname, pid)) == -1){
        fprintf(stderr, "server: Key generate error\n");
        exit(2);
    }

    if((msqid = msgget(key, (IPC_CREAT | 0644))) == -1 ){
        fprintf(stderr, "server: can't get msqid\n");
        exit(2);
    }
    return msqid;
}

struct sockreq listening(int sockfd)
{
    char      msg[BUFLEN];
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    struct sockreq rquest;
    addr_len = sizeof(struct sockaddr_in);

    if (recvfrom(sockfd, msg, BUFLEN , 0,
                (struct sockaddr *) &their_addr, &addr_len) == -1) {
        perror("server: recvfrom");
        exit(1);
    }

    /* TODO: написать от кого */
    printf("server, Получил запрос\n");

    state = REQUEST;

    /* TODO: по-хорошему это надо сделать в check_request */
    rquest.type = 1;
    rquest.their_addr = their_addr;
    rquest.addr_len = addr_len;
    memcpy(&rquest.msg, msg, BUFLEN * sizeof(char));

    return rquest;
}

/* создаем новый сокет */
int create_socket(char *port)
{
    int sockfd;
    int rv;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family,
                             p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket()");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind()");
            continue;
        }

        break;
    }
    freeaddrinfo(servinfo);
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    return sockfd;
}
