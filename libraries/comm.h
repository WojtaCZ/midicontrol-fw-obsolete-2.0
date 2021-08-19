
#ifndef SETUP_H
#define SETUP_H

#include "global.h"

#define COMM_USB_FIFO_SIZE  1024
#define COMM_USART_FIFO_SIZE  1024


void comm_music_songlist(char * param);
void comm_music_play(char * param);
void comm_music_record(char * param);
void comm_music_stop(char * param);
void comm_display_status(char * param);
void comm_display_song(char * param);
void comm_display_verse(char * param);
void comm_display_letter(char * param);
void comm_display_led(char * param);
void comm_base_usb_status(char * param);
void comm_base_isource_status(char * param);
void comm_base_bluetooth_status(char * param);
void comm_base_midi_status(char * param);
void comm_base_midi_config(char * param);


struct cmd_handler{
    char * cmd;
    void (*recv_callback)(char *);
};

#define CMD_COUNT 	14
#define CMD_GET		0x00
#define CMD_SET		0x80

#define MUSIC_SONGLIST			1
#define MUSIC_PLAY				2
#define MUSIC_RECORD			3
#define MUSIC_STOP				4
#define DISPLAY_STATUS			5
#define DISPLAY_SONG			6
#define DISPLAY_VERSE			7
#define DISPLAY_LETTER			8
#define DISPLAY_LED				9
#define BASE_USB_STATUS			10
#define BASE_ISOURCE_STATUS		11
#define BASE_BLUETOOTH_STATUS	12
#define BASE_MIDI_STATUS		13
#define BASE_MIDI_CONFIG		14

void comm_usb_packet_received(uint8_t * buff, int len);

void comm_decode_callback(void);
void comm_timeout_callback(void);
void comm_send_aok(uint8_t aokType, uint8_t recType, uint16_t recSize, uint16_t dataSize, char * msg);
void comm_send_err(uint8_t errType, uint8_t recType, uint16_t recSize);
void comm_send_msg(uint8_t src, uint8_t dest, uint8_t broadcast, uint8_t type, char * msg, uint16_t len);
void comm_cmd_send(uint8_t cmd_type, char * payload);

#endif
