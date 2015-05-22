#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/socket.h> /* constantes, familles... */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* prototypes pour les fonctions dans inet(3N) */
#include <netdb.h>      /* struct hostent */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>

#include "peerfunc.h"


char * filename = NULL;

ssize_t		/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n) //Fd: socket ID, vptr: pointer to buffer
{
    size_t	nleft;
    ssize_t	nread;
    char	*ptr;
    
    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;		/* and call read() again */
            else
                return(-1);
        } else if (nread == 0)
            break;				/* EOF */
        
        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);		/* return >= 0 */
}


ssize_t
Readn(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;
    
    if ( (n = readn(fd, ptr, nbytes)) < 0)
        perror("readn error");
    return(n);
}


/****************************************************************************
 ********  Wrappers pour gerer de maniere rudimentaire les
 ********  erreurs dans les appels systeme
 ****************************************************************************/
void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,
                    void * (*func)(void *), void *arg) {
    int		n;
    
    if ( (n = pthread_create(tid, attr, func, arg)) == 0)
        return;
    errno = n;
    perror("pthread_create error");
    exit(EXIT_FAILURE);
}

void Pthread_join(pthread_t tid, void **status) {
    int		n;
    
    if ( (n = pthread_join(tid, status)) == 0)
        return;
    errno = n;
    perror("pthread_join error");
    exit(EXIT_FAILURE);
}

void Pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr) {
    int		n;
    
    if ( (n = pthread_mutex_init(mptr, attr)) == 0)
        return;
    errno = n;
    perror("pthread_mutex_init error");
    exit(EXIT_FAILURE);
}

void Pthread_mutex_destroy(pthread_mutex_t *mptr) {
    int		n;
    
    if ( (n = pthread_mutex_destroy(mptr)) == 0)
        return;
    errno = n;
    perror("pthread_mutex_destroy error");
    exit(EXIT_FAILURE);
    
}

void Pthread_mutex_lock(pthread_mutex_t *mptr) {
    int		n;
    
    if ( (n = pthread_mutex_lock(mptr)) == 0)
        return;
    errno = n;
    perror("pthread_mutex_lock error");
    exit(EXIT_FAILURE);
}

void Pthread_mutex_unlock(pthread_mutex_t *mptr) {
    int		n;
    
    if ( (n = pthread_mutex_unlock(mptr)) == 0)
        return;
    errno = n;
    perror("pthread_mutex_unlock error");
    exit(EXIT_FAILURE);
}

void Pthread_cancel(pthread_t thread) {
    int		n;
    
    if ( (n = pthread_cancel(thread)) == 0)
        return;
    errno = n;
    perror("pthread_cancel error");
    exit(EXIT_FAILURE);
}
////////////////////////



int nextPiece(struct bitfield * bf, struct beerTorrent *myTorrent, uint32_t * id)
{
    /* Algorithm to select the next piece to download */
    int ok = -1;
    
    for (uint32_t n = 0; n < myTorrent->bf->nbpiece; n++)
    {
        if (isinbitfield(bf, n) && !isinbitfield(myTorrent->bf, n))
        {
            ok = 0;
            *id = n;
            break;
        }
    }
    
    /* Return -1 in case no piece is available, 0 otherwise */
    return ok;
}

void waitAndReply(int sock, struct beerTorrent *myTorrent)
{
    /* Socket is already open */
    
    int connected = 1;
    
    //Initialize peer bitfield to 0
    struct bitfield *peer_bf = (struct bitfield*)malloc(sizeof(struct bitfield));
    peer_bf->nbpiece = myTorrent->bf->nbpiece;
    peer_bf->arraysize = myTorrent->bf->arraysize;
    peer_bf->array = (char*)malloc(sizeof(char) * peer_bf->arraysize);
    for (int i = 0; i < peer_bf->arraysize; i++)
    {
        peer_bf->array[i] = 0;
    }
    
    
    while(connected == 1)
    {
        /* Read message (id and length) */
        struct messageHeader *header = (struct messageHeader *)malloc(sizeof(struct messageHeader));
        int err = readsock(sock, header->length, sizeof(header->length));
        err += readsock(sock, header->id, sizeof(header->id));
        if(err != 0){
            fprintf(stderr, "Cannot read peer message.\n" );
            exit(EXIT_FAILURE);
        }
        
        /* What message is it ?
         - keep-alive: don't disconnect
         - bitfield: update copy of remote bitfield
         - request: send blocks to the other clients
         - piece: add blocks to the file you are downloading !
         - cancel: cancel request */
        if (header->id == 0)
        {
            //Keep-alive
        }
        else if (header->id == 1)
        {
            uint32_t piece_id;
            char temp;
            err = readsock(sock, &temp, sizeof(temp));
            if(err != 0){
                fprintf(stderr, "Cannot read peer message 'HAVE'.\n" );
                exit(EXIT_FAILURE);
            }
            piece_id = atoi(temp);
            //Update bitfield
            setbitinfield(peer_bf, piece_id);
        }
        else if (header->id == 2)
        {
            err = readsock(sock, peer_bf->array, peer_bf->arraysize);
            if(err != 0){
                fprintf(stderr, "Cannot read peer message 'BITFIELD'.\n" );
                exit(EXIT_FAILURE);
            }
            //Update bitfield
            
        }
        else if (header->id == 3)
        {
            //Not needed in minimalist
        }
        else if (header->id == 4)
        {
            //Reading info about the piece
            struct piece *myPiece = (struct piece *)malloc(sizeof(struct piece));
            char temp;
            err = readsock(sock, &temp, sizeof(temp));
            if(err != 0){
                fprintf(stderr, "Cannot read index in peer message 'PIECE'.\n" );
                exit(EXIT_FAILURE);
            }
            myPiece->index = atoi(temp);
            uint32_t piece_id;
            temp;
            err = readsock(sock, &temp, sizeof(temp));
            if(err != 0){
                fprintf(stderr, "Cannot read offset in peer message 'PIECE'.\n" );
                exit(EXIT_FAILURE);
            }
            myPiece->offset = atoi(temp);
            
            //Reading data
            myPiece->data = (char *)malloc(sizeof(char) * myTorrent->piecelength);
            readsock(sock, myPiece->data, sizeof(myPiece->data));
            
            //Writing data to file
            FILE* file = fopen(myTorrent->filename, "w"); /* should check the result */
            if (file == NULL) {
                fprintf(stderr, "Cannot open the file!\n");
                exit(EXIT_FAILURE);
            }
            fseek(file, myPiece->offset, SEEK_SET);
            fwrite(myPiece->data, sizeof(char), sizeof(myPiece->data), file);
            fclose(file);
            
            //Updating the bitfield
            setbitinfield(myTorrent->bf, myPiece->index);
        }
        else if (header->id == 5)
        {
            //Not needed in minimalist
        }
        else
        {
            fprintf(stderr, "Unrecognized peer message.\n" );
            exit(EXIT_FAILURE);
        }
        
        /* Note: if multiple activeThread have been created (to download from multiple clients simultaneously),
         you need a mutex to protect the bitfield and the file ! */
        
        /* Then if needed, make a request to get some blocks / pieces
         Note 1: not needed if originated from passiveThread
         Note 2: use nextPiece to find the next piece you want to download */
        
        /* loop */
    }
}

void *passiveThread(void * f)
{
    /* Initiate handshake */
    
    /* Wait for answer and if everything is OK continue, else exit */
    
    /* Then endless stream */
    
    /* Then endless stream of request / answers in both directions (see: waitAndReply) */
}


void *activeThread(void * params)
{
    /* Get the parameters from params. You need at least:
     - your id
     - your port
     - beerTorrent structure
     - one (remote) peer structure
     - ...
     */
    struct activeParam *myParam = (struct activeParam *)params;
    
    /* Open socket to connect to the (remote) peer */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        fprintf(stderr, "Cannot open socket for peer!\n");
        pthread_exit(NULL);
    }
    struct sockaddr_in peerip;
    peerip.sin_family = AF_INET;
    peerip.sin_port = htons(myParam->myPeer->port);
    struct hostent *temp_struct = (struct hostent*)malloc(sizeof(struct hostent));
    temp_struct = gethostbyaddr(&myParam->myPeer->ipaddr, sizeof(myParam->myPeer->ipaddr), AF_INET);
    memcpy(&(peerip.sin_addr), temp_struct->h_addr_list[0], (size_t)temp_struct->h_length);
    int ret = connect(sock, (struct sockaddr *)&peerip, sizeof(peerip));
    if (ret != 0)
    {
        fprintf(stderr, "Cannot connect to peer!\n");
        close(sock);
        pthread_exit(NULL);
    }
    
    /* Initiate handshake */
    struct handshake *sent = (struct handshake*)malloc(sizeof(struct handshake));
    sent->version = 2;
    strcpy(sent->filehash, myParam->myTorrent->filehash);
    sent->peerId = myParam->myID;
    int err = writesock(sock, &(sent->version), sizeof(sent->version));
    err += writesock(sock, &(sent->filehash), sizeof(sent->filehash));
    err += writesock(sock, &(sent->peerId), sizeof(sent->peerId));
    if (err < 0)
    {
        fprintf(stderr, "Cannot write handshake to peer!\n");
        pthread_exit(NULL);
    }
    
    /* Wait for answer and if everything is OK continue, else exit */
    struct handshake *received = (struct handshake*)malloc(sizeof(struct handshake));
    err = 0;
    err += readsock(sock, received->version, sizeof(received->version));
    err += readsock(sock, received->filehash, sizeof(received->filehash));
    err += readsock(sock, received->peerId, sizeof(received->peerId));
    if (err < 0)
    {
        fprintf(stderr, "Cannot read handshake from peer!\n");
        pthread_exit(NULL);
    }
    
    if (received->version != 2 || strcmp(sent->filehash, received->filehash) != 0)
    {
        fprintf(stderr, "Failure at handshake\n");
        pthread_exit(NULL);
    }
    
    free(sent);
    free(received);

    /* Then endless stream of request / answers in both directions (see: waitAndReply) */
    waitAndReply(sock, myParam->myTorrent);
    
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    srand((unsigned int) time(0));
    /* Parse parameters, such as port and list of beertorrent files */
    int c;
    while ( ( c = getopt(argc,argv,"f:")) != -1) {
        switch (c) {
            case 'f':
                filename = optarg;
                break;
            default:
                fprintf(stderr, "Usage: -f <nom du fichier>\n" );
                exit(EXIT_FAILURE);
        }
    }
    if(filename == NULL){
        fprintf(stderr, "Usage: -f <nom du fichier>\n" );
        exit(EXIT_FAILURE);
    }
    
    /* Generate client id and port */
    u_short port = (u_short) rand() % 100 + 2000;
    uint32_t id = (uint32_t) rand() % 100 + 1;
    printf("My port is %d and my ID is %d.\n", port, id);
    
    
    /* For each beertorrent file, get the beerTorrent structure (tracker, hash ...) */
    struct beerTorrent *myTorrent = addtorrent(filename);
    
    
    printf("Got myTorrent\n");
    /* For each beerTorrent, get the peerList */
    struct peerList *peerlist = gettrackerinfos(myTorrent, id, port);
    
    
    /* Start activeThread per known client to download file
     Note 1: you can debug with one client only
     Note 2: don't connect to yourself ! */
    const int NUM_THREADS = peerlist->nbPeers;
    pthread_t act_thr[NUM_THREADS];
    int i, rc;
    /* create threads */
    for (i = 0; i < NUM_THREADS; ++i) {
        Pthread_create(&act_thr[i], NULL, activeThread, &(peerlist->pentry[i]));
    }
    
    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i) {
        Pthread_join(act_thr[i], NULL);
    }
    
    /* Open socket to listen and wait for new connexions from other clients */
    /*
    while (1)
    {
        //A client attempts to connect, start a passiveThread to handle it
        
        // Continue waiting
    }
    */
    
    
    return EXIT_SUCCESS;
}
