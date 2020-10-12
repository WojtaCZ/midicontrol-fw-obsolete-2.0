#ifndef LED_H
#define LED_H

#include <stdio.h>
#include <unistd.h>

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



void led_setup(void);
void setLEDcolor(uint8_t strip, uint32_t LEDnumber, uint8_t RED, uint8_t GREEN, uint8_t BLUE);

#endif
