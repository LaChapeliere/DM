#include "peerfunc.h"


struct activeParam * activeparam(uint32_t ID, u_short port, struct beerTorrent *torrent, struct peer *p)
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
    fgets(line, sizeof(line), file);
    myTorrent->filelength = atoi(line);
    //File hash
    for (unsigned int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        fscanf(file, "%2hhx", &(myTorrent->filehash[i]));
    }
    fscanf(file, "\n");
    //File name
    fgets(line, sizeof(line), file);
    strcpy(myTorrent->filename, line);
    //Piece length
    fgets(line, sizeof(line), file);
    myTorrent->piecelength = atoi(line);
    //Tracker IP (sockaddr_in)
    myTorrent->trackerip.sin_family = AF_INET;
    myTorrent->trackerip.sin_port = htons(PORTTRACKER);
    struct in_addr tracker;
    fgets(line, sizeof(line), file);
    size_t s = strlen(line) - 1;
    if (line[s] == '\n')
    {
        line[s] = '\0';
    }
    inet_pton(AF_INET, line, &tracker);
    
    struct hostent *temp_struct = (struct hostent*)malloc(sizeof(struct hostent));
    temp_struct = gethostbyaddr(&tracker, sizeof(tracker), AF_INET);
    memcpy (&(myTorrent->trackerip).sin_addr, temp_struct->h_addr_list[0], (size_t)temp_struct->h_length);
    //Pointer to where the file will be reconstructed
    myTorrent->fp = (FILE*)malloc(sizeof(FILE));
    //Bitfield with the hash of the pieces
    myTorrent->bf_hash = (struct bitfield*)malloc(sizeof(struct bitfield));
    myTorrent->bf_hash->nbpiece = ceil(myTorrent->filelength / myTorrent->piecelength);
    myTorrent->bf_hash->arraysize = myTorrent->bf->nbpiece * 20;
    myTorrent->bf_hash->array = (char*)malloc(sizeof(char) * myTorrent->bf_hash->arraysize);
    for (uint32_t n = 0; n < myTorrent->bf->nbpiece; n++)
    {
        char temp[SHA_DIGEST_LENGTH];
        for (unsigned int i = 0; i < SHA_DIGEST_LENGTH; i++)
        {
            fscanf(file, "%2hhx", &temp[i]);
        }
        fscanf(file, "\n");
        strcat(myTorrent->bf_hash->array, temp);
    }
    
    fclose(file);
    //Bitfield with the pieces possessed by client
    myTorrent->bf = (struct bitfield*)malloc(sizeof(struct bitfield));
    myTorrent->bf->nbpiece = ceil(myTorrent->filelength / myTorrent->piecelength);
    myTorrent->bf->arraysize = myTorrent->bf->nbpiece / 8;
    if (myTorrent->bf->nbpiece % 8 != 0)
    {
        myTorrent->bf->arraysize += 1;
    }
    myTorrent->bf->array = (char*)malloc(sizeof(char) * myTorrent->bf->arraysize);
    for (int i = 0; i < myTorrent->bf->arraysize; i++)
    {
        myTorrent->bf->array[i] = 0;
    }
    return myTorrent;
}

struct peerList * gettrackerinfos(struct beerTorrent * bt, uint32_t myId, uint8_t myPort)
{
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
    memcpy(myRequest->fileHash, bt->filehash, sizeof(myRequest->fileHash));
    myRequest->peerId = myId;
    myRequest->port = myPort;
    int err = writesock(sock, myRequest->fileHash, sizeof(myRequest->fileHash));
    err += writesock(sock, &(myRequest->peerId), sizeof(myRequest->peerId));
    err += writesock(sock, &(myRequest->port), sizeof(myRequest->port));
    if (err != 0)
    {
        fprintf(stderr, "Cannot write to tracker!\n");
        exit(EXIT_FAILURE);
    }
    free(myRequest);
    
    printf("Waiting for peerlist from tracker.\n");
    
    struct trackerAnswer *answer = (struct trackerAnswer*)malloc(sizeof(struct trackerAnswer));
    err = 0;
    err += readsock(sock, answer->status, sizeof(answer->status));
    err += readsock(sock, answer->nbPeers, sizeof(answer->nbPeers));
    if (err != 0)
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
        u_char* tmp = (u_char*) malloc (100000);
        readsock(sock, tmp, sizeof(peerId));
        peerId = (uint32_t)atoi((char*)tmp);
        readsock(sock, tmp, sizeof(s_addr));
        s_addr = (uint32_t)atoi((char*)tmp);
        readsock(sock, tmp, sizeof(port));
        port = (uint32_t)atoi((char*)tmp);
        peerlist->pentry[i].peerId = ntohl(peerId);
        peerlist->pentry[i].ipaddr.s_addr = s_addr;
        peerlist->pentry[i].port = ntohs(port);
        free(tmp);
    }
    
    return peerlist;
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
int writesock(int fd, const char * buffer, int len)
{
    int data = 0;
    while (data != len)
    {
        int r = write(fd, (uint8_t*)((u_char*)buffer+data), (len - data));
        if (r == - 1)
        {
            fprintf(stderr, "Error in writesock(): cannot write to socket!\n");
            return -1;
        }
        data += r;
    }
    return 0;
}

int readsock(int fd, char * buffer, int len)
{
    int data = 0;
    while (data != len)
    {
        int r = read(fd, (u_char*)((u_char*)buffer+data), (len - data));
        if (r == - 1)
        {
            fprintf(stderr, "Error in readsock(): cannot read from socket!\n");
            return -1;
        }
        data += r;
    }
    return 0;
}