#include <midi.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/dmamux.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>


void midi_init(void){

    gpio_mode_setup(PORT_USART_MIDI_RX, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_USART_MIDI_RX);
	gpio_mode_setup(PORT_USART_MIDI_TX, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USART_MIDI_TX);
	gpio_set_af(PORT_USART_MIDI_RX, GPIO_AF7, GPIO_USART_MIDI_RX);
	gpio_set_af(PORT_USART_MIDI_TX, GPIO_AF7, GPIO_USART_MIDI_TX);

	usart_set_baudrate(USART1, 31250);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_CR2_STOPBITS_1);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_enable(USART1);

}