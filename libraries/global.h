#ifndef GLOBAL_H
#define GLOBAL_H

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libopencm3/stm32/gpio.h>

#define PORT_FRONT_LED	        GPIOB
#define GPIO_FRONT_LED	        GPIO14

#define PORT_BACK_LED	        GPIOB
#define GPIO_BACK_LED	        GPIO5

#define PORT_USB_DP 	        GPIOA
#define GPIO_USB_DP 	        GPIO12

#define PORT_USB_DM 	        GPIOA
#define GPIO_USB_DM 	        GPIO11

#define PORT_I2C1_SDA 	        GPIOB
#define GPIO_I2C1_SDA 	        GPIO7

#define PORT_I2C1_SCL 	        GPIOA
#define GPIO_I2C1_SCL	        GPIO15

#define PORT_USART_MIDI_TX 	    GPIOB
#define GPIO_USART_MIDI_TX 	    GPIO10

#define PORT_USART_MIDI_RX 	    GPIOB
#define GPIO_USART_MIDI_RX 	    GPIO11

#define PORT_ENCODER            GPIOB
#define GPIO_ENCODER_A          GPIO0
#define GPIO_ENCODER_A_IRQ      NVIC_EXTI0_IRQ
#define GPIO_ENCODER_B          GPIO2
#define GPIO_ENCODER_B_IRQ      NVIC_EXTI2_IRQ
#define GPIO_ENCODER_SW         GPIO1
#define GPIO_ENCODER_SW_IRQ     NVIC_EXTI1_IRQ


#endif
