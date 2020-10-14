#ifndef USB_H
#define USB_H

#include <setup.h>

void usb_setup(void);

uint32_t usb_cdc_tx(void *buf, int len);
uint32_t usb_midi_tx(void *buf, int len);

#endif
