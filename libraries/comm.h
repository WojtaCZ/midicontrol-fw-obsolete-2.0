
#ifndef SETUP_H
#define SETUP_H

#include "global.h"

#define COMM_USB_FIFO_SIZE  1024
#define COMM_USART_FIFO_SIZE  1024


void comm_usb_packet_received(uint8_t * buff, int len);

void comm_decode_callback(void);
void comm_send_aok(uint8_t aokType, uint8_t recType, uint16_t recSize, uint16_t dataSize, char * msg);
void comm_send_err(uint8_t errType, uint8_t recType, uint16_t recSize);
void comm_send_msg(uint8_t src, uint8_t dest, uint8_t broadcast, uint8_t type, char * msg, uint16_t len);


#endif
