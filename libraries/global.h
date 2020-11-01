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




//Definice pro typy zprav
#define ADDRESS_MAIN 				0x02
#define ADDRESS_PC 					0x00
#define ADDRESS_CONTROLLER 			0x01

#define ADDRESS_OTHER 				0x03

#define AOKERR 						0x07
#define ERR 						0x00
#define AOK 						0x80

#define INTERNAL 					0x01
#define EXTERNAL_BT 				0x02
#define EXTERNAL_DISP 				0x03
#define EXTERNAL_MIDI				0x04
#define EXTERNAL_USB 				0x05

#define INTERNAL_COM 				0x00
#define INTERNAL_COM_KEEPALIVE 		0xAB
#define INTERNAL_COM_STOP 			0x00
#define INTERNAL_COM_PLAY 			0x01
#define INTERNAL_COM_REC 			0x02
#define INTERNAL_COM_CHECK_NAME 	0x03
#define INTERNAL_COM_GET_SONGS 		0x04
#define INTERNAL_COM_GET_TIME 		0x05
#define INTERNAL_COM_SET_TIME 		0x06

#define INTERNAL_DISP 				0x01
#define INTERNAL_DISP_GET_STATUS 	0x00
#define INTERNAL_DISP_SET_SONG	 	0x01
#define INTERNAL_DISP_SET_VERSE		0x02
#define INTERNAL_DISP_SET_LETTER 	0x03
#define INTERNAL_DISP_SET_LED	 	0x04

#define INTERNAL_USB 				0x02
#define INTERNAL_USB_GET_CONNECTED	0x00

#define INTERNAL_CURR 				0x03
#define INTERNAL_CURR_GET_STATUS	0x00
#define INTERNAL_CURR_SET_STATUS 	0x01

#define INTERNAL_BT		 			0x04
#define INTERNAL_BT_GET_STATUS		0x00

#define INTERNAL_MIDI 				0x05
#define INTERNAL_MIDI_GET_STATUS	0x00

#endif
