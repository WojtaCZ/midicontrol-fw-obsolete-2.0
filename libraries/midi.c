#include "midi.h"
#include "usb.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/dmamux.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>

#include <string.h>

uint8_t midiFifo[100], midiFifoIndex, midiGotMessage;

void midi_init(void){

    gpio_mode_setup(PORT_USART_MIDI_RX, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USART_MIDI_RX);
	gpio_mode_setup(PORT_USART_MIDI_TX, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_USART_MIDI_TX);
	gpio_set_af(PORT_USART_MIDI_RX, GPIO_AF7, GPIO_USART_MIDI_RX);
	gpio_set_af(PORT_USART_MIDI_TX, GPIO_AF7, GPIO_USART_MIDI_TX);


	//Prijimani DMA
	dma_set_priority(DMA1, DMA_CHANNEL4, DMA_CCR_PL_VERY_HIGH);
	dma_set_memory_size(DMA1, DMA_CHANNEL4, DMA_CCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL4, DMA_CCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL4);
	dma_set_read_from_peripheral(DMA1, DMA_CHANNEL4);

    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL4);
    nvic_enable_irq(NVIC_DMA1_CHANNEL4_IRQ);

	dmamux_set_dma_channel_request(DMAMUX1, DMA_CHANNEL4, DMAMUX_CxCR_DMAREQ_ID_UART3_RX);

	dma_set_peripheral_address(DMA1, DMA_CHANNEL4, (uint32_t)&USART3_RDR);
	dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[0]);
    dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);

	//Vysilani DMA
	dma_set_priority(DMA1, DMA_CHANNEL5, DMA_CCR_PL_VERY_HIGH);
	dma_set_memory_size(DMA1, DMA_CHANNEL5, DMA_CCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL5, DMA_CCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL5);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL5);

	dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL5);

	dmamux_set_dma_channel_request(DMAMUX1, DMA_CHANNEL5, DMAMUX_CxCR_DMAREQ_ID_UART3_TX);

	usart_set_baudrate(USART3, 31250);
	usart_set_databits(USART3, 8);
	usart_set_parity(USART3, USART_PARITY_NONE);
	usart_set_stopbits(USART3, USART_STOPBITS_1);
	usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART3, USART_MODE_TX_RX);
	usart_enable_rx_dma(USART3);
	dma_enable_channel(DMA1, DMA_CHANNEL4);
	
	usart_enable(USART3);
}

void midi_send(uint8_t * buff, int len){
	dma_set_peripheral_address(DMA1, DMA_CHANNEL5, (uint32_t)&USART3_TDR);
	dma_set_memory_address(DMA1, DMA_CHANNEL5, (uint32_t)buff);
    dma_set_number_of_data(DMA1, DMA_CHANNEL5, len);
	usart_enable_tx_dma(USART3);
	nvic_enable_irq(NVIC_DMA1_CHANNEL5_IRQ);
	dma_enable_channel(DMA1, DMA_CHANNEL5);
}

void dma1_channel5_isr(){
	dma_disable_channel(DMA1, DMA_CHANNEL5);
	usart_disable_tx_dma(USART3);
	dma_clear_interrupt_flags(DMA1, DMA_CHANNEL5, DMA_TCIF);
    nvic_clear_pending_irq(NVIC_DMA1_CHANNEL5_IRQ);
}


void dma1_channel4_isr(){
	dma_disable_channel(DMA1, DMA_CHANNEL4);
	usart_disable_rx_dma(USART3);

	//Precte se typ zpravy
	uint8_t msgType = midiFifo[0];

	//Pokud prijde byte validni MIDI zpravy
	if((msgType & 0xF0) >= 0x80 && !midiGotMessage){
		midiGotMessage = 1;

		//Zpravy co maji 2 byty
		if((msgType >= 0x80 && msgType <= 0xBF) || (msgType & 0xF0) == 0xE0 || msgType == 0xF2 || msgType == 0xF0){
			dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex]);
    		dma_set_number_of_data(DMA1, DMA_CHANNEL4, 2);
			midiFifoIndex += 2;
		}else if((msgType & 0xF0) == 0xC0 ||  (msgType & 0xF0) == 0xD0 || msgType == 0xF3){
			dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex++]);
    		dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);
		}else{
			midiFifoIndex = 0;
			dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex++]);
    		dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);
		}


	}else if(midiGotMessage){
		//Pokud byla zprava sysex a prisel sysex end
		if(msgType == 0xF0 && midiFifo[midiFifoIndex-1] == 0xF7){
			midiFifoIndex = 0;
			midiFifo[midiFifoIndex] = 0;
			midiGotMessage = 0;
			dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex++]);
    		dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);
		}else if(msgType == 0xF0 && midiFifo[midiFifoIndex-1] != 0xF7){
			//Pokud byla zprava sysex a prisly data
			dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex++]);
    		dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);
		}else{
			//Ostatni zpravy
			uint8_t buffer[4];
			//Vynuluje se buffer
			memset(buffer,0,4);
			//Vytvori se CN a CIN
			buffer[0] = ((midiFifo[0] >> 4) & 0x0F);
			//Data se presunou do bufferu
			memcpy(&buffer[1], &midiFifo[0], midiFifoIndex);


			usb_midi_tx(buffer, 4);
			usb_cdc_tx(buffer, 4);
			//Zacne se novy prijem
			midiFifoIndex = 0;
			midiGotMessage = 0;
			midiFifo[midiFifoIndex] = 0;
			dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex++]);
    		dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);
		}
	}else{
		//Pokud nema validni MIDI zpravu, prijima dal
		midiGotMessage = 0;
		midiFifoIndex = 0;
		midiFifo[midiFifoIndex] = 0;
		dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)&midiFifo[midiFifoIndex++]);
    	dma_set_number_of_data(DMA1, DMA_CHANNEL4, 1);
	}


	dma_clear_interrupt_flags(DMA1, DMA_CHANNEL4, DMA_TCIF);
    nvic_clear_pending_irq(NVIC_DMA1_CHANNEL4_IRQ);
	usart_enable_rx_dma(USART3);
	dma_enable_channel(DMA1, DMA_CHANNEL4);
	usart_enable(USART3);
}

