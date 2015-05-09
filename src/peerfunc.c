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
    sha_hexa_to_bin(myTorrent->filehash, line);
    //File name
    fgets(line, sizeof(line), file);
    strcpy(myTorrent->filename, line);
    //Piece length
    fgets(line, sizeof(line), file);
    myTorrent->piecelength = atoi(line);
    //Tracker IP
    fgets(line, sizeof(line), file);
    strcpy(myTorrent->trackerip, line);
    //Pointer to where the file will be reconstructed
    myTorrent->fp = (FILE*)malloc(sizeof(FILE));
    //Bitfield
    myTorrent->bf = (struct bitfield*)malloc(sizeof(struct bitfield));
    myTorrent->bf->nbpiece = ceil(myTorrent->filelength / myTorrent->piecelength);
    myTorrent->bf->arraysize = myTorrent->bf->nbpiece * 20;
    myTorrent->bf->array = (char*)malloc(sizeof(char) *myTorrent->bf->arraysize);
    for (int n = 0; n < myTorrent->bf->nbpiece; n++)
    {
        fgets(line, sizeof(line), file);
        char temp[20];
        sha_hexa_to_bin(temp, line);
        strcat(myTorrent->bf->array, temp);
    }
    
    fclose(file);
    return myTorrent;
}

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