#ifndef SETUP_H
#define SETUP_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libopencm3/stm32/gpio.h>

#define PORT_FRONT_LED	GPIOB
#define GPIO_FRONT_LED	GPIO14

#define PORT_BACK_LED	GPIOB
#define GPIO_BACK_LED	GPIO5

#define PORT_USB_DP 	GPIOA
#define GPIO_USB_DP 	GPIO12

#define PORT_USB_DM 	GPIOA
#define GPIO_USB_DM 	GPIO11

void clock_setup(void);

#endif
