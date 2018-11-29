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

#define RTT_MS 200

#define KEY1 0x9A
#define KEY2 0xB8

#define DATA 0
#define ACK 1

// PDU structures
typedef struct {char tag; } tag_t;

typedef struct {
    tag_t type;
    int   seal;
    char  str[0];
} data_pdu_t;

typedef struct {
    tag_t type;
    int   seal;
} ack_pdu_t;

typedef union {
    char raw[FRAME_PAYLOAD_SIZE];

    tag_t        pdu_type;
    data_pdu_t   data;
    ack_pdu_t    ack;
} pdu_frame_t;

#define DATA_PAYLOAD_SIZE (FRAME_PAYLOAD_SIZE - sizeof(data_pdu_t));

int fingerprint(char * str, int key) {
    char sum = 0;
    char * s;
    for (s = str; *s != 0; s++ ){ sum ^= *s; }
    return ((int) sum) ^ key;
}

int ecg_init(int address){
  return radio_init(address);
}

int ecg_send(int dst, char* msg, int len, int to_ms){
  pdu_frame_t buf;
  alarm_t timer1;
  int src, time_left, err;
  int done = 0;

  alarm_init(&timer1);

  if (dst < 1024 || dst > 0xFFFF || len > FRAME_PAYLOAD_SIZE || to_ms <= 0){
    return ERR_INVAL;
  }
  // Start overall timer
  alarm_set(&timer1, to_ms);

  // Flush pending frames
  while ( (err=radio_recv(&src, buf.raw, 0)) >= ERR_OK) {
      printf("Flushing pending frame of size %d\n", err);
  };

  if (err != ERR_TIMEOUT) { return ERR_FAILED; }



  while (!done && !alarm_expired(&timer1)) {

      /* INIT STATE */

      // Prepare buffer seen as DATA PDU
      buf.data.type.tag = DATA;
      buf.data.seal = fingerprint(msg, KEY1);
      strcpy(buf.data.str, msg);                // Includes null terminator

      // Send the message
      if ( (err=radio_send(dst, buf.raw, len)) != ERR_OK) {
          printf("radio_send failed with %d\n", err);
          return ERR_FAILED;
      }

      /* WAITING STATE */

      while (1) {
          time_left = alarm_rem(&timer1);
          if (time_left > RTT_MS) { time_left = RTT_MS; }

          err=radio_recv(&src, buf.raw, time_left);
          if (err>=ERR_OK) {

              // Somehting received -- check if expected acknowledgement
              if (err != sizeof(ack_pdu_t) || buf.pdu_type.tag != ACK) {
                  // Not an ACK packet -- ignore
                  printf("Non-ACK packcet with length %d received\n", err);
                  continue;
              }

              // Check sender
              if (src != dst) {
                  printf("Wrong sender: %d\n", src);
                  continue;
              };

              // Check fingerprint
              if (buf.ack.seal != fingerprint(msg, KEY2)) {
                  printf("Wrong fingerprint: 0x%08x\n", buf.ack.seal);
                  continue;
              };

              // ACK ok
              done = 1;
              break;
          }

          if (err!=ERR_TIMEOUT) {
              // Something went wrong
              return err;
          }

          // Timeout - try to send again
          break;
      }
  }

  /* DONE STATE */

  return done ? ERR_OK : ERR_TIMEOUT;



}


int ecg_recv(int* src, char* msg, int len, int to_ms) {

    pdu_frame_t buf;
    int err;
//    char msg[80];

    while (1) {

        /* READY STATE */

        while (1) {

            err=radio_recv(&src, buf.raw, -1);
            if (err>=ERR_OK) {

                // Somehting received -- check if DATA PDU
                if (err < sizeof(data_pdu_t) || buf.pdu_type.tag != DATA) {
                    // Not a DATA packet -- ignore
                    printf("Non-DATA packcet with length %d received\n", err);
                    continue;
                }

                // Check message consistency and fingerprint
                if ( sizeof(data_pdu_t) + strlen(buf.data.str) + 1 != err) {
                    printf("Length mismatch: %d\n", err);
                    continue;
                }
                strcpy(msg, buf.data.str);
                if (buf.data.seal != fingerprint(msg, KEY1)) {
                    printf("Wrong fingerprint: 0x%08x\n", buf.data.seal);
                    continue;
                };

                // DATA PDU ok
                break;
            }

            printf("Unexpected error %d\n", err);
            exit(1);
        }

        /* ACKNOWLEDGE STATE */

        // Prepare frame seen as ACK PDU
        buf.ack.type.tag = ACK;
        buf.ack.seal = fingerprint(msg, KEY2);

        // Send acknowledgement to sender
        if ( (err=radio_send(src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK) {
            printf("radio_send failed with %d\n", err);
            return ERR_FAILED;
        }

        /* DONE STATE */

        printf("Received message from %d: %s\n", src, msg);

    }
}
