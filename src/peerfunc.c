#include "peerfunc.h"

/* BeerTorrent */
struct beerTorrent * addtorrent(const char * filename);

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