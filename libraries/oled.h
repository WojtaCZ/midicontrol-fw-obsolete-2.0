#ifndef OLED_H
#define OLED_H

#include <libopencm3/stm32/gpio.h>

#define PORT_FRONT_LED	GPIOB
#define GPIO_FRONT_LED	GPIO14

#define PORT_BACK_LED	GPIOB
#define GPIO_BACK_LED	GPIO5

void oled_setup(void);

#endif
