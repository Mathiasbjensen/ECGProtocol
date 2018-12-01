/*
 * ecg.h
 *
 * Interface to the ECG communication node
 */

#ifndef _ECG_H_
#define _ECG_H_

#include "errors.h"

int ecg_init(int addr);

int ecg_send(int dst, char* msg, int len);

int ecg_recv();

#endif // _ECG_H_
