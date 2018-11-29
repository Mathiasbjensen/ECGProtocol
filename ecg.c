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

#define INIT 0
#define WAIT_ACK_REQUEST 1
#define WAIT_ACK 2
#define SEND_DATA 3
#define READY 0
#define WAIT_PACK 1

#define BUFFLEN 72

int ecg_init(int address){
  return radio_init(address);
}

// PDU structures
typedef struct {char tag; } tag_t;

typedef struct {
    tag_t type;
    char  str[0];
} req_pdu_t;

typedef struct {
    tag_t type;
    char  str[0];
} data_pdu_t;

typedef struct {
    tag_t type;
} ack_pdu_t;

typedef union {
    char raw[FRAME_PAYLOAD_SIZE];

    tag_t        pdu_type;
    data_pdu_t   data;
    ack_pdu_t    ack;
    req_pdu_t    req;
} pdu_frame_t;

int ecg_send(int dst){
  int state=INIT;
  pdu_frame_t data;
  char msg[FRAME_PAYLOAD_SIZE];
  int err;
  char lenstr[10];
  int source, packN, len;

  while(1) {

    switch(state){
      case INIT:
            printf("Enter message: ");
            fgets(msg, FRAME_PAYLOAD_SIZE, stdin);

            if (strlen(msg) > 72) {
              len = ceil(strlen(msg)/72);
              sprintf(lenstr, "%d", len-1);

            } else {
              sprintf(lenstr, "%d", 1);
            }

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
            printf("Waiting to recieve ACK \n");
            if ( (len=radio_recv(&source, data.raw, -1))==ERR_OK) {
                printf("radio_recv failed with %d\n", len);
                return 1;
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
            memcpy(data.data.str,&msg[BUFFLEN*packN],BUFFLEN);
            printf("data send: %s\n", data.data.str);
            if ( (err=radio_send(dst, data.raw, sizeof(data_pdu_t)) != ERR_OK)) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }
            printf("%d\n", data.data.type.tag);
            state = WAIT_ACK_REQUEST;
            break;
    }

  }

  return -1;
}

int ecg_recv(){
  int state = READY;
  pdu_frame_t buf;
  int source, len, err;
  char data[FRAME_PAYLOAD_SIZE];
  int lenOfReq, lenOfData, packN;

  while(1) {
    switch (state) {
      case READY:
            if ( (len=radio_recv(&source, buf.raw, -1))==ERR_OK) {
                /* if (len == ERR_TIMEOUT) {
                    printf("radio_recv timed out\n");
                    continue;
                } */
                printf("radio_recv failed with %d\n", len);
                return 1;
              }

            if ( buf.pdu_type.tag != REQ) {
              printf("Received something else than REQ\n");
              return ERR_FAILED;
            }
            lenOfData = atoi(buf.req.str);

            printf("Received REQ length of file is %d\n",lenOfData);

            buf.ack.type.tag = ACK;

            if ( (err=radio_send(source, buf.raw, sizeof(pdu_frame_t))) != ERR_OK) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }
            printf("Send ack to %d\n",source);
            packN = 0;
            state = WAIT_PACK;
            break;
      case WAIT_PACK:
            if ( (len=radio_recv(&source, buf.raw, -1))==ERR_OK) {
                printf("radio_recv failed with %d\n", len);
                return 1;
              }
            printf("%d\n",buf.data.type.tag);
            printf("%d\n",buf.pdu_type.tag);
            if ( buf.pdu_type.tag != DATA) {
              printf("Received something else than DATA\n");
              return ERR_FAILED;
            }
            if (packN == lenOfData) {
              int j = 0;
              while (buf.data.str[j] != '\0') {
                data[packN*BUFFLEN + j] = buf.data.str[j];
              }
            } else {
              memcpy(&data[BUFFLEN*packN],buf.data.str, BUFFLEN);
            }

            printf("data so far: %s", data);

    }

  }

  return -1;
}
