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
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


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
    peer_bf->array = (u_char*)malloc(sizeof(u_char) * peer_bf->arraysize);
    for (int i = 0; i < (int)peer_bf->arraysize; i++)
    {
        peer_bf->array[i] = 0;
    }
    
    
    while(connected == 1)
    {
        /* Read message (id and length) */
        struct messageHeader *header = (struct messageHeader *)malloc(sizeof(struct messageHeader));
        int err = readsock(sock, (char*)&(header->length), sizeof(header->length));
        header->length = ntohl(header->length);
        err += readsock(sock, (char*)&(header->id), sizeof(header->id));
        if(err <= 0){
            fprintf(stderr, "Cannot read peer message.\n" );
            exit(EXIT_FAILURE);
        }
        
        /* What message is it ?
         - keep-alive: don't disconnect
         - bitfield: update copy of remote bitfield
         - request: send blocks to the other clients
         - piece: add blocks to the file you are downloading !
         - cancel: cancel request */
        printf("Message = %d\n", header->id);
        if (header->id == 0)
        {
            //Keep-alive
        }
        else if (header->id == 1)
        {
            uint32_t piece_id;
            err = readsock(sock, (char*)&piece_id, sizeof(piece_id));
            if(err <= 0){
                fprintf(stderr, "Cannot read peer message 'HAVE'.\n" );
                exit(EXIT_FAILURE);
            }
            piece_id = ntohl(piece_id);
            //Update bitfield
            setbitinfield(peer_bf, piece_id);
        }
        else if (header->id == 2)
        {
            err = readsock(sock, (char*)peer_bf->array, sizeof(peer_bf->array));
            if(err <= 0){
                fprintf(stderr, "Cannot read peer message 'BITFIELD'.\n" );
                exit(EXIT_FAILURE);
            }
        }
        else if (header->id == 3)
        {
            //Not needed in minimalist
            fprintf(stderr, "Message 'REQUEST' not implemented.\n" );
        }
        else if (header->id == 4)
        {
            //Reading info about the piece
            struct piece *myPiece = (struct piece *)malloc(sizeof(struct piece));
            err = readsock(sock, (char*)&(myPiece->index), sizeof(myPiece->index));
            if(err <= 0){
                fprintf(stderr, "Cannot read index in peer message 'PIECE'.\n" );
                exit(EXIT_FAILURE);
            }
            myPiece->index = ntohl(myPiece->index);
            err = readsock(sock, (char*)&(myPiece->offset), sizeof(myPiece->offset));
            if(err <= 0){
                fprintf(stderr, "Cannot read offset in peer message 'PIECE'.\n" );
                exit(EXIT_FAILURE);
            }
            myPiece->offset = ntohl(myPiece->offset);
            
            //Reading data
            myPiece->data = (char *)malloc(sizeof(char) * myTorrent->piecelength);
            err = readsock(sock, myPiece->data, sizeof(myPiece->data));
            if(err <= 0){
                fprintf(stderr, "Cannot read data in peer message 'PIECE'.\n" );
                exit(EXIT_FAILURE);
            }
            
            //Critical section
            Pthread_mutex_lock(&mutex);
            
            //Writing data to file
            FILE* file = fopen(myTorrent->filename, "w"); /* should check the result */
            if (file == NULL) {
                fprintf(stderr, "Cannot open the file!\n");
                exit(EXIT_FAILURE);
            }
            fseek(file, myPiece->offset + myPiece->index * myTorrent->piecelength, SEEK_SET);
            fwrite(myPiece->data, sizeof(char), sizeof(myPiece->data), file);
            fclose(file);
            
            //Updating the bitfield
            setbitinfield(myTorrent->bf, myPiece->index);
            
            //End of critical section
            Pthread_mutex_unlock(&mutex);
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
        
        printf("MESSAGE OK\n");
        
        /* Note: if multiple activeThread have been created (to download from multiple clients simultaneously),
         you need a mutex to protect the bitfield and the file ! */
        
        /* Then if needed, make a request to get some blocks / pieces
         Note 1: not needed if originated from passiveThread
         Note 2: use nextPiece to find the next piece you want to download */
        Pthread_mutex_lock(&mutex);
        struct requestPayload *request = (struct requestPayload*)malloc(sizeof(struct requestPayload));
        int full = nextPiece(peer_bf, myTorrent, &(request->index));
        request->offset = 0;
        request->length = myTorrent->piecelength;
        if (full == -1)
        {
            //There is no piece left to load from this client
            connected = 0;
        }
        else
        {
            printf("Request piece %d\n", request->index);
            struct messageHeader * header = (struct messageHeader*)malloc(sizeof(struct messageHeader));
            header->length = 13;
            header->length = htonl(header->length);
            header->id = '3';
            int err = writesock(sock, &(header->length), sizeof(header->length));
            err += writesock(sock, &(header->id), sizeof(header->id));
            request->index = htonl(request->index);
            request->offset = htonl(request->offset);
            request->length = htonl(request->length);
            err += writesock(sock, &(request->index), sizeof(request->index));
            err += writesock(sock, &(request->offset), sizeof(request->offset));
            err += writesock(sock, &(request->length), sizeof(request->length));
            if(err != 0){
                fprintf(stderr, "Cannot write message 'REQUEST'.\n" );
                exit(EXIT_FAILURE);
            }
            free(header);
        }
        free(request);
        Pthread_mutex_unlock(&mutex);
        /* loop */
    }
}

void *passiveThread(void * f)
{
    /* Initiate handshake */
    
    /* Wait for answer and if everything is OK continue, else exit */
    
    /* Then endless stream */
    
    /* Then endless stream of request / answers in both directions (see: waitAndReply) */
    pthread_exit(NULL);
}


void *activeThread(void * params)
{
    int r = rand() % 100;
    /* Get the parameters from params. You need at least:
     - your id
     - your port
     - beerTorrent structure
     - one (remote) peer structure
     - ...
     */
    struct activeParam *myParam = (struct activeParam *)params;
    
    /* Open socket to connect to the (remote) peer */
    struct sockaddr_in peerip;
    memset(&peerip, 0, sizeof(peerip));
    peerip.sin_family = AF_INET;
    peerip.sin_port = htons(myParam->myPeer->port);
    printf("%d\n", myParam->myPeer->port);
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(myParam->myPeer->ipaddr), str, INET_ADDRSTRLEN);
    printf("%s\n", str);
    peerip.sin_addr = myParam->myPeer->ipaddr;
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        fprintf(stderr, "Cannot open socket for peer %d\n", myParam->myPeer->peerId);
        pthread_exit(NULL);
    }
    int ret = connect(sock, (struct sockaddr *)&peerip, sizeof(peerip));
    if (ret != 0)
    {
        fprintf(stderr, "Cannot connect to peer %s!\n", str);
        close(sock);
        pthread_exit(NULL);
    }
    printf("Connected to peer %s.\n", str);
    
    /* Initiate handshake */
    struct handshake *sent = (struct handshake*)malloc(sizeof(struct handshake));
    sent->version = 2;
    strcpy(sent->filehash, myParam->myTorrent->filehash);
    sent->peerId = htonl(myParam->myID);
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
    err += readsock(sock, (char*)&(received->version), sizeof(received->version));
    err += readsock(sock, received->filehash, sizeof(received->filehash));
    err += readsock(sock, (char*)&(received->peerId), sizeof(received->peerId));
    received->peerId = ntohl(received->peerId);
    if (received->version != 2 || strncmp(sent->filehash, received->filehash, SHA_DIGEST_LENGTH) != 0)
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
    uint16_t port = (uint16_t) rand() % 100 + 2000;
    uint32_t id = (uint32_t) rand() % 4294967294 + 1;
    printf("My port is %d and my ID is %d.\n", port, id);
    
    
    /* For each beertorrent file, get the beerTorrent structure (tracker, hash ...) */
    struct beerTorrent *myTorrent = addtorrent(filename);
    
    /* For each beerTorrent, get the peerList */
    struct peerList *peerlist = gettrackerinfos(myTorrent, id, port);
    printf("Got peerList\n");
    
    /* Start activeThread per known client to download file
     Note 1: you can debug with one client only
     Note 2: don't connect to yourself ! */
    const int NUM_THREADS = peerlist->nbPeers;
    pthread_t act_thr[NUM_THREADS];
    int i, rc;
    /* create threads */
    Pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < NUM_THREADS; ++i) {
        printf("Creating the active thread for %d-th peer\n", i);
        struct activeParam *params = activeparam(id, port, myTorrent, &(peerlist->pentry[i]));
        Pthread_create(&act_thr[i], NULL, activeThread, params);
    }
    
    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i) {
        Pthread_join(act_thr[i], NULL);
    }
    
    //NOT FOR MINIMALIST
    /* Open socket to listen and wait for new connexions from other clients */
    /*
    while (1)
    {
        //A client attempts to connect, start a passiveThread to handle it
        
        // Continue waiting
    }
    */
    
    Pthread_mutex_destroy(&mutex);
    
    printf("END!\n");
    
    return EXIT_SUCCESS;
}
