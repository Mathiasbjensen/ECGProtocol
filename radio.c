/*
 * radio.c
 *
 * Emulation of radio node using UDP (skeleton)
 */

// Implements
#include "radio.h"

// Uses
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#define RADIOADDR "127.0.0.1"
#define BUFLEN 72

int sock;    // UDP Socket used by this node


void die(char *s)
{
        perror(s);
        exit(1);
}

int radio_init(int addr) {
    struct sockaddr_in sa;   // Structure to set own address


    // Check validity of address
    if(addr <= 0xFFFF && addr >= 1024){ // skal vi tjekke om unsigned?
      printf("Address ok!");
    }
    else {
      printf("Address NOT ok!");
      die("address");
    }

    // Create UDP socket
    if ( (sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
      die("socket");
    }

    // Prepare address structure
    memset((char *) &sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(addr);
    if (inet_aton(RADIOADDR , &sa.sin_addr) == 0)
    {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
    }

    // Bind socket to port
    if( bind(sock , (struct sockaddr*)&sa, sizeof(sa) ) == -1)
    {
      die("bind");
    }

    return ERR_OK;
}

int radio_send(int  dst, char* data, int len) {


    struct sockaddr_in sa;   // Structure to hold destination address
    int slen=sizeof(sa);

    // Check that port and len are valid
    if (len>BUFLEN || dst < 1024){
      die("port or bufferlength");
    }
    len = len + 22;
    // Emulate transmission time
    sleep(len/100);

    // Prepare address structure
    memset((char *) &sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(dst);
    if (inet_aton(RADIOADDR , &sa.sin_addr) == 0)
    {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
    }

    // Send the message & check if it's fully sent.
    // Check blocking af sendto
    if (sendto(sock, data, len , 0 , (struct sockaddr *) &sa, slen)!=len)
    {
        die("sendto()");
    }

    return ERR_OK;
}

int radio_recv(int* src, char* data, int to_ms) {

    char buf[BUFLEN];
    struct sockaddr_in sa;   // Structure to receive source address
    int slen=sizeof(sa);

    int len = -1;            // Size of received packet (or error code)

    memset((char *) &sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(*src);
    if (inet_aton(RADIOADDR , &sa.sin_addr) == 0)
    {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
    }


    // First poll/select with timeout (may be skipped at first)


    // Receive packet/data
    len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &sa, &slen);
    if (len == -1)
    {
            die("recvfrom()");
    }

    for (int i=0; i<BUFLEN; i++){
      if(buf[i]=='\0'){
        break;
      }
      data[i]=buf[i];

    }
    // Set source from address structure

    *src = ntohs(sa.sin_port);
    return len;
}
