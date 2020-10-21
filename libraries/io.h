#ifndef IO_H
#define IO_H

#include "global.h"

void io_init(void);
void io_encoder_read(void);
void io_sw_read(void);

void io_encoder_push(void);
void io_encoder_dec(void);
void io_encoder_inc(void);

#endif