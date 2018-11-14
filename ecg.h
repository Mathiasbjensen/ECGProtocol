/*
 * ecg.h
 *
 * Interface to the ECG communication node
 */

#ifndef _ECG_H_
#define _ECG_H_

#include "errors.h"

int ecg_init(int addr);

int ecg_send(int  dst, char* packet, int len, int to_ms);

int ecg_recv(int* src, char* packet, int len, int to_ms);

#endif // _ECG_H_
