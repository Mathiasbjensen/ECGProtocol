#include "radio.h"
#include "ecg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "alarm.h"
#include <time.h>

#define DATA 3
#define ACK 1
#define REQ 2

//Sender states
#define INIT 0
#define WAIT_ACK_REQUEST 1
#define WAIT_ACK 2
#define SEND_DATA 3
#define DONE 4

//Reciver states
#define READY 0
#define WAIT_PACK 1
#define SEND_ACK 2

#define BUFFLEN 71
#define SEND_BUF_SIZE 1024

#define LAST_CHAR '|'

int ecg_init(int address){
  return radio_init(address);
}

// PDU structures
typedef struct {char tag; } tag_t;

typedef struct {
    tag_t type;
    char  str[0];
    int checksum;
} req_pdu_t;

typedef struct {
    tag_t type;
    char  str[0];
    int checksum;
} data_pdu_t;

typedef struct {
    tag_t type;
    int checksum;
} ack_pdu_t;

typedef union {
    char raw[FRAME_PAYLOAD_SIZE];

    tag_t        pdu_type;
    data_pdu_t   data;
    ack_pdu_t    ack;
    req_pdu_t    req;
} pdu_frame_t;

int ecg_send(int dst, char* msg, int len){
  int state=INIT;
  pdu_frame_t data;
  //char msg[SEND_BUF_SIZE];
  int err;
  int packagesLost = 0;
  char lenstr[10];
  int source, packN, lastmsglen;

  while(1) {

    switch(state){
      case INIT:

            msg[len] = '\0';
            lastmsglen = (len > BUFFLEN) ? len%BUFFLEN : len;
            len = (len > BUFFLEN) ? len/BUFFLEN+1 : 1;

            printf("Length %d string %s",len,msg);
            sprintf(lenstr, "%d", len);

            data.req.type.tag = REQ;
            strcpy(data.data.str, lenstr);
            printf("Sending: %s with length %d\n",data.data.str,strlen(data.data.str));
            if ( (err=radio_send(dst, data.raw, strlen(lenstr))) != ERR_OK) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }
            printf("send REQ \n");
            state = WAIT_ACK_REQUEST;
            break;

      case WAIT_ACK_REQUEST:
            printf("Waiting to recieve REQ ACK \n");
            if ( (err=radio_recv(&source, data.raw, -1))==ERR_OK) {
                printf("radio_recv failed with %d\n", err);
                return 1;
              }
            if(err==ERR_TIMEOUT){
              packagesLost++;
              state = DONE;
              break;
            }
            if (data.pdu_type.tag != ACK){
              printf("Received something else than ACK\n");
              return ERR_FAILED;

            }
            packN = 0;
            printf("Recieved ACK \n");
            state = SEND_DATA;
            break;
      case SEND_DATA:
            data.data.type.tag = DATA;
            if(packN == len-1){
              memcpy(data.data.str,&msg[(BUFFLEN)*packN], lastmsglen);
              //printf("Lastmsglen: %d\n", lastmsglen);
              data.data.str[lastmsglen] = LAST_CHAR;
              //memset(&data.data.str[lastmsglen],'\0',BUFFLEN-lastmsglen);
              //printf("hej %s\n", data.data.str);
            } else {
              memcpy(data.data.str,&msg[(BUFFLEN)*packN],BUFFLEN);
            }
            printf("data sent: %s\n", data.data.str);
            if ( (err=radio_send(dst, data.raw, FRAME_PAYLOAD_SIZE) != ERR_OK)) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }
            printf("pack nr: %d length: %d\n", packN, len);
            packN++;

            state = WAIT_ACK;
            break;
      case WAIT_ACK:
            printf("Waiting to recieve ACK \n");
            if ( (err=radio_recv(&source, data.raw, -1))==ERR_OK) {
                printf("radio_recv failed with %d\n", err);
                return ERR_FAILED;
              }

            if(err==ERR_TIMEOUT){
              packagesLost++;
              state = DONE;
              break;
            }

            if (data.pdu_type.tag != ACK){
              printf("Received something else than ACK\n");
              return ERR_FAILED;

            }
            printf("Recieved ACK \n");

            state = (packN == len) ? DONE : SEND_DATA;
            break;
      case DONE:
            printf("Data sent. %d packages lost",packagesLost);
            return 1;
    }

  }

  return -1;
}

int ecg_recv(){
  int state = READY;
  pdu_frame_t buf;
  int source, len, err, last;
  char data[SEND_BUF_SIZE] = {};
  int lenOfReq, lenOfData, packN;
  int packagesLost = 0;

  while(1) {
    switch (state) {
      case READY:
            printf("Ready to recieve data\n");
            if ( (len=radio_recv(&source, buf.raw, -1))==ERR_OK) {
                /* if (len == ERR_TIMEOUT) {
                    printf("radio_recv timed out\n");
                    continue;
                } */
                printf("radio_recv failed with %d\n", len);
                return ERR_FAILED;
              }

            if(err==ERR_TIMEOUT){
              packagesLost++;
              state = DONE;
              break;
            }

            if ( buf.pdu_type.tag != REQ) {
              printf("Received something else than REQ\n");
              return ERR_FAILED;
            }
            lenOfData = atoi(buf.req.str);

            printf("Received REQ length of file is %d\n",lenOfData);

            buf.ack.type.tag = ACK;

            if ( (err=radio_send(source, buf.raw, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }
            printf("Send ack to %d\n",source);
            packN = 0;
            state = WAIT_PACK;
            break;
      case WAIT_PACK:
            //memset(&buf.data.str[0],'\0', sizeof(buf.data.str));
            if ( (len=radio_recv(&source, buf.raw, -1))==ERR_OK) {
                printf("radio_recv failed with %d\n", len);
                return 1;
              }
            if(err==ERR_TIMEOUT){
              packagesLost++;
              state = DONE;
              break;
            }
            //printf("buf.data.str: %s   - END\n",buf.data.str);
            last = strlen(buf.data.str) - 1;
            buf.data.str[last] = LAST_CHAR;   // Drop ending newline

            if ( buf.pdu_type.tag != DATA) {
              printf("Received something else than DATA\n");
              return ERR_FAILED;
            }
            if (packN == lenOfData-1) {
              int j = 0;
              while (buf.data.str[j] != LAST_CHAR) {
                //printf("%c\n",buf.data.str[j]);
                data[packN*BUFFLEN + j] = buf.data.str[j];
                j++;
              }
              printf("j: %i\n", j);
              //memset(&data[packN*BUFFLEN],'\0',BUFFLEN);

            } else {
              memcpy(&data[BUFFLEN*packN],buf.data.str, BUFFLEN);
            }
            packN++;
            state = SEND_ACK;
            break;
      case SEND_ACK:
            buf.ack.type.tag = ACK;

            if ( (err=radio_send(source, buf.raw, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }
            printf("Send ack to %d\n",source);

            state = (packN == lenOfData) ? DONE : WAIT_PACK;
            break;

      case DONE:
            printf("%d packages lost. Result = %s",packagesLost,data);
            return 1;


    }

  }

  return -1;
}
