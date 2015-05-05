#include "peerfunc.h"

/* BeerTorrent */
struct beerTorrent * addtorrent(const char * filename)
{
    struct beerTorrent *myTorrent = (struct beerTorrent*)malloc(sizeof(struct beerTorrent));
    
    //Opening the file to parse info
    FILE* file = fopen(filename, "r"); /* should check the result */
    char line[256];
    //File size
    fgets(line, sizeof(line), file);
    myTorrent->filelength = atoi(line);
    //File hash
    fgets(line, sizeof(line), file);
    strcpy(myTorrent->filehash, line);
    //File name
    fgets(line, sizeof(line), file);
    strcpy(myTorrent->filename, line);
    //Piece length
    fgets(line, sizeof(line), file);
    myTorrent->piecelength = atoi(line);
    //Tracker IP
    
    //Pointer to where the file will be reconstructed
    
    //
    
    fclose(file);
}
/*
struct beerTorrent
{
    uint32_t filelength;
    char filehash[SHA_DIGEST_LENGTH];
    char filename[MAXNAMELENGTH];
    uint32_t piecelength;
    struct in_addr trackerip;
    FILE * fp;
    struct bitfield * bf;
};
*/
struct peerList * gettrackerinfos(struct beerTorrent * bt, uint32_t myId, uint8_t myPort);

/* Bitfield */
struct bitfield * createbitfield(uint32_t filelength, uint32_t piecelength);

void destroybitfield(struct bitfield * bf);

void setbitfield(struct bitfield * dst, struct bitfield * src);

void setbitinfield(struct bitfield * bf, uint32_t id);

int isinbitfield(struct bitfield * bf, uint32_t id);

/* Sockets */
int writesock(int fd, const char * buf, int len);

int readsock(int fd, char * buffer, int len);