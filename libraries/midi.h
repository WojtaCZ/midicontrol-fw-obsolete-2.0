#ifndef MIDI_H
#define MIDI_H

#include "global.h"


void midi_init(void);
void midi_send(uint8_t * buff, int len);

#endif