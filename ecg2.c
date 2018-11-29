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

int ecg_init(int address){
  return radio_init(address);
}

void ecg_send(){
  int state=init;
  char data[FRAME_PAYLOAD_SIZE];

  while(1) {

    switch(state){
      case init:
            printf("Enter message: ");
            fgets(data, FRAME_PAYLOAD_SIZE, stdin);
            len = strlen(data);
            req_start = "REQ"+len;
            if ( (err=radio_send(dst, req_start, strlen(req_start))) != ERR_OK) {
                printf("radio_send req failed with %d\n", err);
                return ERR_FAILED;
            }


    }
  }
}

void ecg_recv(){
  int state = ready;
  char buf[FRAME_PAYLOAD_SIZE + 1];
  int source, len;
  char data[FRAME_PAYLOAD_SIZE + 1];
  int lenOfReq;

  while(1) {
    switch (state) {
      case ready:
            if ( (len=radio_recv(&source, buf, TIMEOUT_SEC * 1000)) < 0) {
                /* if (len == ERR_TIMEOUT) {
                    printf("radio_recv timed out\n");
                    continue;
                } */
                printf("radio_recv failed with %d\n", len);
                return 1;
              }
            buf[len] = '\0';
            if (buf[0]=='R' && buf[1] == 'E' && buf[2] == 'Q'){
                int i = 3;
                while (buf[i] != '\0'){
                   data[i-3] = buf[i];
                   i++;
                }
                sscanf(data,"%d",&lenOfReq);
                printf("%s\n", lenOfReq);

            }

    }
  }
}
