/*
 * radio.h
 *
 * Interface to the radio device
 */

#ifndef _RADIO_H_
#define _RADIO_H_

#include "errors.h"

#define FRAME_PAYLOAD_SIZE     72

int radio_init(int addr);

int radio_send(int  dst, char* data, int len);

int radio_recv(int* src, char* data, int to_ms);

#endif // _RADIO_H_
