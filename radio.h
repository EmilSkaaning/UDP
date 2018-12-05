/*
 * radio.h
 *
 * Interface to the radio device
 */

#ifndef _RADIO_H_
#define _RADIO_H_

#include "errors.h"

#define PREAMBLE                10
#define MAGICKEY                 4
#define HEADER                   6
#define PAYLOAD                 72
#define CKECKSUM                 2
#define FRAME_PAYLOAD_SIZE     184

int radio_init(int addr);

int radio_send(int  dst, char* data, int len);

int radio_recv(int* src, char* data, int to_ms);

int magickey_gen();

#endif // _RADIO_H_
