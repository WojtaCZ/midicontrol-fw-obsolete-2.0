#ifndef LED_H
#define LED_H

#include "global.h"

#define LED_STRIP_BACK				0
#define LED_STRIP_FRONT				1

#define LED_BACK_NUMBER				6
#define LED_FRONT_NUMBER			4

#define LED_REQ_FREQ					(800000)
#define LED_TIMER_CLOCK_FREQ			(144e6)
#define LED_TIMER_PERIOD				(LED_TIMER_CLOCK_FREQ / LED_REQ_FREQ)
#define LED_RESET_SLOTS_BEGIN			(200)
#define LED_RESET_SLOTS_END				(200)
#define LED_LAST_SLOT					(1)
#define LED_0							(LED_TIMER_PERIOD / 3)
#define LED_1							(LED_TIMER_PERIOD * 2 / 3)
#define LED_RESET						(0)

#define LED_BACK_DATA_SIZE				(LED_BACK_NUMBER * 24)
#define LED_BACK_BUFFER_SIZE			(LED_RESET_SLOTS_BEGIN + LED_BACK_DATA_SIZE + LED_LAST_SLOT + LED_RESET_SLOTS_END)

#define LED_FRONT_DATA_SIZE				(LED_FRONT_NUMBER * 24)
#define LED_FRONT_BUFFER_SIZE			(LED_RESET_SLOTS_BEGIN + LED_FRONT_DATA_SIZE + LED_LAST_SLOT + LED_RESET_SLOTS_END)

//Definice pro device LED

#define PERIF_COUNT	            (4 | 0x80)

#define LED_CLR_OK		        0x004000
#define LED_CLR_ERROR	        0x400000
#define LED_CLR_DATA	        0x000040
#define LED_CLR_LOAD	        0x404000

#define LED_CLR_GREEN		    0x004000
#define LED_CLR_RED			    0x400000
#define LED_CLR_BLUE		    0x000040
#define LED_CLR_YELLOW		    0x404000
#define LED_CLR_CLEAR		    0x000000

#define LED_STATUS_ERR 		    0
#define LED_STATUS_OK 			1
#define LED_STATUS_DATA 		2
#define LED_STATUS_LOAD 		3
#define LED_STATUS_CLR			4

#define LED_DEV_USB				0
#define LED_DEV_DISP			1
#define LED_DEV_CURRENT			2
#define LED_DEV_MIDIA			3
#define LED_DEV_MIDIB			4
#define LED_DEV_BLUETOOTH		5

#define LED_FRONT1				(0 | 0x80)
#define LED_FRONT2				(1 | 0x80)
#define LED_FRONT3				(2 | 0x80)
#define LED_FRONT4				(3 | 0x80)


void led_init(void);
void led_set_color(uint8_t strip, uint32_t LEDnumber, uint8_t RED, uint8_t GREEN, uint8_t BLUE);
void led_set_strip_color(uint8_t strip, uint8_t RED, uint8_t GREEN, uint8_t BLUE);
void led_fill_buff_black(uint8_t strip);
void led_fill_buff_white(uint8_t strip);

void led_dev_set_status(uint8_t perif, uint8_t status);
void led_dev_set_status_all(uint8_t strip, uint8_t status);
void led_dev_set_color(uint8_t perif, uint32_t color);
void led_dev_set_color_all(uint8_t strip, uint32_t color);
void led_dev_process_pending_status(void);

#endif
