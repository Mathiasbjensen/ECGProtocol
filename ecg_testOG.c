/*
 * ecg_test.c
 *
 * Simple test of ECG protocol
 * Parameters:
 *   -r     Runs as receiver (default)
 *   -s     Runs as sender
 *   -ra p  Set recevier address to p (port)
 *   -sa p  Set sender address to p (port)
 */


// Uses
#include "ecg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SEND_TIMEOUT_SEC        5
#define RECV_TIMEOUT_SEC       10
#define SEND_BUF_SIZE        1024
#define RECV_BUF_SIZE         256

int is_sender = 0;
int snd_addr = 2132;
int rcv_addr = 2135;

void read_args(int argc, char * argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0) { is_sender = 0;  continue; }
        if (strcmp(argv[i], "-s") == 0) { is_sender = 1;  continue; }
        if (strcmp(argv[i], "-sa") == 0 && i < argc - 1) {
            snd_addr = atoi(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "-ra") == 0 && i < argc - 1) {
            rcv_addr = atoi(argv[++i]);
            continue;
        }
        printf("Unknown option %s\n", argv[i]);
    }
}

int sender() {
    int err, last;

    char msg[SEND_BUF_SIZE];

    printf("Acting as sender with address %d sending to %d\n", snd_addr, rcv_addr);

    if ( (err=ecg_init(snd_addr)) != ERR_OK) {
        printf("Protocol could not be initialized: %d\n", err);
        return 1;
    }

    printf("Protocol node initialized\n");

    while (1) {

        // Get next message from console
        printf("Enter message: ");
        printf("venter");
        fgets(msg, SEND_BUF_SIZE, stdin);
        printf("venter ikke");
        last = strlen(msg) - 1;
        if (msg[last] == '\n') { msg[last] = '\0'; }  // Drop ending newline
        printf("et eller nadet");
        // Send it reliably
        err = ecg_send(rcv_addr, msg, strlen(msg), SEND_TIMEOUT_SEC * 1000);

        if (err != ERR_OK && err != ERR_TIMEOUT) {
            printf("ecg_send failed with %d\n", err);
            continue;
        }

        if (err == ERR_TIMEOUT) {
            printf("... timed out\n");
            continue;
        }

        printf("Reliably sent: %s\n", msg);
    }

    return 0;
}


int receiver() {
    int err, len, source;

    char buf[RECV_BUF_SIZE];

    printf("Acting as receiver with address %d\n", rcv_addr);

    if ( (err=ecg_init(rcv_addr)) != ERR_OK) {
        printf("Protocol could not be initialized: %d\n", err);
        return 1;
    }

    printf("Protocol init initialized\n");

    while (1) {

        if ( (len=ecg_recv(&source, buf, sizeof(buf), RECV_TIMEOUT_SEC * 1000)) < 0) {
            if (len == ERR_TIMEOUT) {
                printf("ecg_recv timed out\n");
                continue;
            }
            printf("ecg_recv failed with %d\n", len);
            continue;
        }

        buf[len] = '\0';
        printf("Received %d bytes from address %d: %s\n", len, source, buf);
    }

    printf("Receive loop ended!\n");
    return 1;

}


int main(int argc, char * argv[]) {

    read_args(argc, argv);

    return is_sender ? sender() :  receiver();

}
