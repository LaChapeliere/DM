#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <string.h>
#include <math.h>
#include <arpa/inet.h>
#include "additionalFunctions.h"



//#define SHA_DIGEST_LENGTH 161
#define PORTTRACKER 3955
#define MAXNAMELENGTH 128

/* Tracker communication structures */
struct clientRequest
{
    char fileHash[SHA_DIGEST_LENGTH];
    uint32_t peerId;
    uint8_t port;
};

struct trackerAnswer
{
    u_char status;
    u_char nbPeers;
};

struct peer
{
    uint32_t peerId;
    struct in_addr ipaddr;
    uint8_t port;
};

struct peerList
{
    u_char nbPeers;
    struct peer * pentry;
};

/* Inter-client communication structures */
struct handshake
{
    u_char version;
    char filehash[SHA_DIGEST_LENGTH];
    uint32_t peerId;
};

struct messageHeader
{
    uint32_t length;
    u_char id;
};

struct requestPayload
{
    uint32_t index;
    uint32_t offset;
    uint32_t length;
};

struct piece
{
    uint32_t index;
    uint32_t offset;
    char *data;
};

/* Beertorrent struct */
struct bitfield
{
    u_char * array;
    uint32_t arraysize;
    uint32_t nbpiece;
};

struct beerTorrent
{
    uint32_t filelength;
    char filehash[SHA_DIGEST_LENGTH];
    char filename[MAXNAMELENGTH];
    uint32_t piecelength;
    struct sockaddr_in trackerip;
    FILE * fp;
    struct bitfield * bf_hash; //For bonus where you can start loading a file again after an interruption
    struct bitfield * bf;
};

/*Parameter struct for activeThread*/
struct activeParam
{
    uint32_t myID;
    u_short myport;
    struct beerTorrent *myTorrent;
    struct peer *myPeer;
};

struct activeParam * activeparam(uint32_t ID, u_short port, struct beerTorrent *torrent, struct peer *p);


/* BeerTorrent */
struct beerTorrent * addtorrent(const char * filename);

struct peerList * gettrackerinfos(struct beerTorrent * bt, uint32_t myId, uint8_t myPort);

int hostname_to_ip(char *hostname , char *ip);

/* Bitfield */
struct bitfield * createbitfield(uint32_t filelength, uint32_t piecelength);

void destroybitfield(struct bitfield * bf);

void setbitfield(struct bitfield * dst, struct bitfield * src);

void setbitinfield(struct bitfield * bf, uint32_t id);

int isinbitfield(struct bitfield * bf, uint32_t id);

/* Sockets */
int writesock(int fd, const char * buf, int len);

int readsock(int fd, char * buffer, int len);
