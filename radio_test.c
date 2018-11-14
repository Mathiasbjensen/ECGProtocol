/*
 * radio_test.c
 *
 * Test of simple radio device interface
 * Parameters:
 *   -r     Runs as receiver (default)
 *   -s     Runs as sender
 *   -ra p  Set recevier address to p (port)
 *   -sa p  Set sender address to p (port)
 */


// Uses
#include "radio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TIMEOUT_SEC        5
#define BUF_SIZE          60

int is_sender = 0;
int snd_addr = 2132;
int rcv_addr = 2135;

/* For larger number of options, use a library, e.g. gopt */
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

    char data[FRAME_PAYLOAD_SIZE];

    printf("Acting as sender with address %d\n", snd_addr);

    if ((err=radio_init(snd_addr)) != ERR_OK) {
        printf("Radio could not be initialized: %d\n", err);
        return 1;
    }

    printf("Radio node initialized\n");

    // Prepare first message
    strcpy(data, "Hello from sender!");

    while (1) {

        if ( (err=radio_send(rcv_addr, data, strlen(data))) != ERR_OK) {
            printf("radio_send failed with %d\n", err);
            return 1;
        }

        printf("Sent: %s\n", data);

        // Get next message from console
        printf("Enter message: ");
        fgets(data, FRAME_PAYLOAD_SIZE, stdin);
        last = strlen(data) - 1;
        if (data[last] == '\n') { data[last] = '\0'; }  // Drop ending newline
    }

    return 0;
}


int receiver() {

    char buf[FRAME_PAYLOAD_SIZE + 1];   // Add room for string termination

    int err, len;
    int source;

    printf("Acting as receiver with address %d\n", rcv_addr);

    if ((err=radio_init(rcv_addr)) != ERR_OK) {
        printf("Radio could not be initialized: %d\n", err);
        return 1;
    }

    printf("Radio node init initialized\n");

    while (1) {

        if ( (len=radio_recv(&source, buf, TIMEOUT_SEC * 1000)) < 0) {
            if (len == ERR_TIMEOUT) {
                printf("radio_recv timed out\n");
                continue;
            }
            printf("radio_recv failed with %d\n", len);
            return 1;
        }

        buf[len] = '\0';
        printf("Received from address %d: %s\n", source, buf);
    }


    return 0;
}


int main(int argc, char * argv[]) {

    read_args(argc, argv);

    return is_sender ? sender() :  receiver();

}
