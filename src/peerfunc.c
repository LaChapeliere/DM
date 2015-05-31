#include "peerfunc.h"


struct activeParam * activeparam(uint32_t ID, uint16_t port, struct beerTorrent *torrent, struct peer *p)
{
    struct activeParam *param = (struct activeParam*)malloc(sizeof(struct activeParam));
    
    param->myID = ID;
    param->myport = port;
    param->myTorrent = torrent;
    param->myPeer = p;
    
    return param;
}


/* BeerTorrent */
struct beerTorrent * addtorrent(const char * filename)
{
    struct beerTorrent *myTorrent = (struct beerTorrent*)malloc(sizeof(struct beerTorrent));
    
    //Opening the file to parse info
    FILE* file = fopen(filename, "r"); /* should check the result */
    if (file == NULL) {
        fprintf(stderr, "Cannot open .beertorrent!\n");
        exit(EXIT_FAILURE);
    }
    char line[256];
    //File size
    fscanf(file, "%d\n", &myTorrent->filelength);
    //File hash
    for (unsigned int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        fscanf(file, "%2hhx", &myTorrent->filehash[i]);
    }
    
    //File name
    fscanf(file, "%s\n", myTorrent->filename);
    //Piece length
    fscanf(file, "%d\n", &myTorrent->piecelength);
    //Tracker IP (sockaddr_in)
    memset(&(myTorrent->trackerip), 0, sizeof(myTorrent->trackerip));
    myTorrent->trackerip.sin_family = AF_INET;
    myTorrent->trackerip.sin_port = htons(PORTTRACKER);
    fscanf(file, "%s\n", line);
    char *ip = (char*)malloc(sizeof(char) * 256);
    if (isalpha(line[0]))
    {
        hostname_to_ip(line, ip);
    }
    else if (isdigit(line[0]))
    {
        strcpy(ip, line);
    }
    else
    {
        fprintf(stderr, "Invalid tracker ip!\n");
    }
    int er = inet_aton(ip, &(myTorrent->trackerip.sin_addr));
    if (er != 1)
    {
        fprintf(stderr, "Could not convert IP to binary!\n");
        fprintf(stderr, "Error code : %d\n", er);
        fprintf(stderr, "Input : %s\n", line);
        exit(EXIT_FAILURE);
    }
    free(ip);
    
    //Pointer to where the file will be reconstructed
    myTorrent->fp = (FILE*)malloc(sizeof(FILE));
    
    fclose(file);
    //Bitfield with the pieces possessed by client
    myTorrent->bf = (struct bitfield*)malloc(sizeof(struct bitfield));
    myTorrent->bf->nbpiece = ceil(myTorrent->filelength / myTorrent->piecelength);
    myTorrent->bf->arraysize = myTorrent->bf->nbpiece / 8;
    if (myTorrent->bf->nbpiece % 8 != 0)
    {
        myTorrent->bf->arraysize += 1;
    }
    myTorrent->bf->array = (u_char*)malloc(sizeof(u_char) * myTorrent->bf->arraysize);
    for (int i = 0; i < (int)myTorrent->bf->arraysize; i++)
    {
        myTorrent->bf->array[i] = 0;
    }
    return myTorrent;
}

struct peerList * gettrackerinfos(struct beerTorrent * bt, uint32_t myId, uint16_t myPort)
{
    printf("Test gettrackerinfos\n");
    //Connect to tracker
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        fprintf(stderr, "Cannot open socket for tracker!\n");
        exit(EXIT_FAILURE);
    }
    int ret = connect(sock, (struct sockaddr *)&(bt->trackerip), sizeof(bt->trackerip));
    if (ret != 0)
    {
        fprintf(stderr, "Cannot connect to tracker!\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    struct clientRequest *myRequest = (struct clientRequest*)malloc(sizeof(struct clientRequest));
    memcpy(myRequest->fileHash, bt->filehash, sizeof(bt->filehash));
    myRequest->peerId = htonl(myId);
    myRequest->port = htons(myPort);
    int err = 0;
    for (unsigned int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        printf("%02hhx", myRequest->fileHash[i]);
    }
    printf("\n");
    err += writesock(sock, (uint8_t *) &(myRequest->fileHash), sizeof(myRequest->fileHash));
    err += writesock(sock, (uint8_t *) &(myRequest->peerId), sizeof(myRequest->peerId));
    err += writesock(sock, (uint8_t *) &(myRequest->port), sizeof(myRequest->port));
    if (err != 0)
    {
        fprintf(stderr, "Cannot write to tracker!\n");
        exit(EXIT_FAILURE);
    }
    free(myRequest);
    
    printf("Waiting for peerlist from tracker.\n");
    
    struct trackerAnswer *answer = (struct trackerAnswer*)malloc(sizeof(struct trackerAnswer));
    err = 0;
    err += readsock(sock, (char*)&(answer->status), sizeof(answer->status));
    err += readsock(sock, (char*)&(answer->nbPeers), sizeof(answer->nbPeers));
    if (err <= 0)
    {
        fprintf(stderr, "Cannot read from tracker!\n");
        exit(EXIT_FAILURE);
    }
    printf("Tracker: status %d, %d peers.\n", answer->status, answer->nbPeers);
    if (answer->status != 0)
    {
        fprintf(stderr, "Tracker cannot find file!\n");
        exit(EXIT_FAILURE);
    }
    
    struct peerList *peerlist = (struct peerList*)malloc(sizeof(struct peerList));
    peerlist->nbPeers = answer->nbPeers;
    peerlist->pentry = (struct peer*)malloc(sizeof(struct peer) * peerlist->nbPeers);
    
    for (unsigned int i = 0; i < peerlist->nbPeers; i++)
    {
        uint32_t peerId;
        uint32_t s_addr;
        uint16_t port;
        
        err = readsock(sock, (char*)&peerId, sizeof(peerId));
        err += readsock(sock, (char*)&s_addr, sizeof(s_addr));
        err += readsock(sock, (char*)&port, sizeof(port));
        if (err <= 0)
        {
            fprintf(stderr, "Cannot read from tracker!\n");
            exit(EXIT_FAILURE);
        }
        
        peerlist->pentry[i].peerId = ntohl(peerId);
        peerlist->pentry[i].ipaddr.s_addr = s_addr;
        peerlist->pentry[i].port = ntohs(port);
    }
    return peerlist;
}

int hostname_to_ip(char *hostname , char *ip)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
    
    if ( (rv = getaddrinfo( hostname , "http" , &hints , &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        h = (struct sockaddr_in *) p->ai_addr;
        strcpy(ip , inet_ntoa( h->sin_addr ) );
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

/* Bitfield */
struct bitfield * createbitfield(uint32_t filelength, uint32_t piecelength);

void destroybitfield(struct bitfield * bf);

void setbitfield(struct bitfield * dst, struct bitfield * src);

void setbitinfield(struct bitfield * bf, uint32_t id)
{
    bf->nbpiece += (u_int)!isinbitfield(bf,id) ;
    bf->array[id/8] |= (u_char)(0x1 << (id%8));
}

int isinbitfield(struct bitfield * bf, uint32_t id)
{
    return !!(bf->array[id/8] & (0x1 << (id%8)));
}

/* Sockets */

int writesock(int fd, const void * buffer, int len)
{
    int data = 0;
    while (data != len)
    {
        int er = write(fd, (char*)buffer + data, (len - data));
        if (er == -1)
        {
            fprintf(stderr, "Error in writesock(): cannot write to socket!\n");
            return -1;
        }
        data += er;
    }
    return 0;
}

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
        ptr += nread;
    }
    return(n - nleft);		/* return >= 0 */
}


int readsock(int fd, char * buffer, int len)
{
    ssize_t n;
    
    if ( (n = readn(fd, buffer, (size_t)len)) < 0)
    {
        fprintf(stderr, "Error in readsock(): cannot read from socket!\n");
    }
    return(n);
}
