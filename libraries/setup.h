#ifndef SETUP_H
#define SETUP_H

#include <libopencm3/stm32/gpio.h>

#define PORT_FRONT_LED	GPIOB
#define GPIO_FRONT_LED	GPIO14

#define PORT_BACK_LED	GPIOB
#define GPIO_BACK_LED	GPIO5

void clock_setup(void);

#endif
