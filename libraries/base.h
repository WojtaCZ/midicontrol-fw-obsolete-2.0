
#ifndef BASE_H
#define BASE_H

#include "global.h"
#include "menu.h"

void base_init(void);
void base_set_current_source(uint8_t state);
int base_get_current_source();
void base_menu_set_current_source(void * m);

void base_record(uint8_t initiator, char * songname);
void base_play(Menu * itm);
void base_stop(uint8_t initiator);
void base_req_songlist();

#endif
