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

int nextPiece(struct bitfield * bf, uint32_t * id)
{
    /* Algorithm to select the next piece to download */

    /* Return -1 in case no piece is available, 0 otherwise */
}

void waitAndReply(/* ... */)
{
  /* Socket is already open */

  int connected = 1;

  while(connected == 1)
  {
    /* Read message (id and length) */

    /* What message is it ? 
      - keep-alive: don't disconnect
      - bitfield: update copy of remote bitfield
      - request: send blocks to the other clients
      - piece: add blocks to the file you are downloading !
      - cancel: cancel request */

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


  /* Open socket to connect to the (remote) peer */


  /* Initiate handshake */
  
  /* Wait for answer and if everything is OK continue, else exit */

  
  /* Initialize the bitfield of your beerTorrent file (all zero if you don't have any piece) */

  /* Send the bitfield message (first message sent to the other client) 
     Note: The other client should also send his bitfield, you will get it in the next function */
  
  /* Then endless stream of request / answers in both directions (see: waitAndReply) */
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
    u_int id = (u_int) rand() % 100 + 1;
    printf("My port is %d and my ID is %d.\n", port, id);
    
  
  /* For each beertorrent file, get the beerTorrent structure (tracker, hash ...) */
    struct beerTorrent *myTorrent = addtorrent(filename);
  
  /* For each beerTorrent, get the peerList */
    //Connection au tracker
    char port_tracker[]="3955";
    
    struct addrinfo hints; // structure pour faire la demande
    struct addrinfo *result, *rp; // structure pour stocker et lire les résultats
    int s=-1; // socket  (s)
    int res; // variables pour tester si les fonctions donnent un résultats ou une erreur
    int bon;
    // Des variable pour contenir de adresse de machine et des numero de port afin de les afficher
    char hname[NI_MAXHOST], sname[NI_MAXSERV];
    
    
    // on rempli la structure hints de demande d'adresse
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* IPv4 ou IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* socket flux connectée */
    hints.ai_flags = 0;    /* Les signifie que toutes les addresse de la machine seront utilisée */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    
    res = getaddrinfo(myTorrent->trackerip, port_tracker, &hints, &result);
    if (res != 0) { // c'est une erreur
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    // si res = 0 le véritable résultat de la fontion est l'argument result
    // qui contient une liste d'addresse correspondant à la demande on va les
    // rester jusqu'a trouver une qui convient
    rp = result;
    bon = 0;
    while (rp != NULL) {
        // on parcourt la liste pour en trouver une qui convienne
        
        // on récupère des informations affichables
        res = getnameinfo(rp->ai_addr, rp->ai_addrlen,
                          hname, NI_MAXHOST,
                          sname, NI_MAXSERV,
                          NI_NUMERICSERV|NI_NUMERICHOST);
        if (res != 0) {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(res));
            exit (1);
        }
        fprintf (stderr, "On tente l'adresse %s sur le port %s .....\n",
                 hname, sname);
        
        // on essaye
        s = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        // si le résultat est -1 cela n'a pas fonctionné on recommence avec la prochaine
        if (s == -1) {
            perror("Création de la socket");
            rp = rp->ai_next;
            continue;
        }
        
        // si la socket a été obtenue, on essaye de se connecter
        res = connect(s, rp->ai_addr, rp->ai_addrlen);
        if (res == 0 ) {// cela a fonctionné on est connecté
            bon = 1;
            fprintf (stderr, "OK\n");
            break;
        }
        else { // sinon le bind a été impossible, il faut fermer la socket
            perror("Impossible de se connecter");
            close (s);
            s=-1;
        }
        
        rp = rp->ai_next;
    }
    
    freeaddrinfo(result);           /* No longer needed */
    
    if (bon == 0) { // Cela n'a jamais fonctionné
        fprintf(stderr, "Aucune connexion possible\n");
    } else {
        
    }
    
  
  /* Start activeThread per known client to download file
     Note 1: you can debug with one client only
     Note 2: don't connect to yourself ! */

  /* Open socket to listen and wait for new connexions from other clients */
  
  while (1)
  {
      /* A client attempts to connect, start a passiveThread to handle it */
      
      /* Continue waiting */
  }
  
  return EXIT_SUCCESS;
}
