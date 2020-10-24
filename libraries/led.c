#include <led.h>
#include <setup.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/dmamux.h>
#include <libopencm3/cm3/nvic.h>


uint8_t ledFrontBuffer[LED_FRONT_BUFFER_SIZE];
uint8_t ledBackBuffer[LED_BACK_BUFFER_SIZE];

uint32_t led_pending_flags[LED_BACK_NUMBER];
uint32_t led_statuses[LED_BACK_NUMBER];


void led_init(){

	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_FRONT_LED | GPIO_BACK_LED);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO_FRONT_LED | GPIO_BACK_LED);
	gpio_set_af(GPIOB, GPIO_AF10, GPIO_BACK_LED);
	gpio_set_af(GPIOB, GPIO_AF1, GPIO_FRONT_LED);

	//Setup for back led
	dma_set_priority(DMA1, DMA_CHANNEL1, DMA_CCR_PL_LOW);
	dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL1);
	dma_enable_circular_mode(DMA1, DMA_CHANNEL1);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL1);

	dmamux_set_dma_channel_request(DMAMUX1, DMA_CHANNEL1, DMAMUX_CxCR_DMAREQ_ID_TIM17_CH1);

	dma_set_peripheral_address(DMA1, DMA_CHANNEL1, (uint32_t)&TIM17_CCR1);
	dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t)&ledBackBuffer);
	dma_set_number_of_data(DMA1, DMA_CHANNEL1, LED_BACK_BUFFER_SIZE);

	timer_enable_preload(TIM17);
	timer_update_on_overflow(TIM17);
	timer_set_dma_on_update_event(TIM17);
	timer_enable_irq(TIM17, TIM_DIER_CC1DE);
	timer_generate_event(TIM17, TIM_EGR_CC1G);
	timer_set_oc_mode(TIM17, TIM_OC1, TIM_OCM_PWM1);
	timer_enable_oc_output(TIM17, TIM_OC1);
	timer_enable_break_main_output(TIM17);
	timer_set_period(TIM17, LED_TIMER_PERIOD-1);

	timer_enable_counter(TIM17);
	dma_enable_channel(DMA1, DMA_CHANNEL1);

	//Setup for front led
	dma_set_priority(DMA1, DMA_CHANNEL2, DMA_CCR_PL_LOW);
	dma_set_memory_size(DMA1, DMA_CHANNEL2, DMA_CCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL2, DMA_CCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL2);
	dma_enable_circular_mode(DMA1, DMA_CHANNEL2);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL2);

	dmamux_set_dma_channel_request(DMAMUX1, DMA_CHANNEL2, DMAMUX_CxCR_DMAREQ_ID_TIM15_CH1);

	dma_set_peripheral_address(DMA1, DMA_CHANNEL2, (uint32_t)&TIM15_CCR1);
	dma_set_memory_address(DMA1, DMA_CHANNEL2, (uint32_t)&ledFrontBuffer);
	dma_set_number_of_data(DMA1, DMA_CHANNEL2, LED_FRONT_BUFFER_SIZE);

	timer_enable_preload(TIM15);
	timer_update_on_overflow(TIM15);
	timer_set_dma_on_update_event(TIM15);
	timer_enable_irq(TIM15, TIM_DIER_CC1DE);
	timer_generate_event(TIM15, TIM_EGR_CC1G);
	timer_set_oc_mode(TIM15, TIM_OC1, TIM_OCM_PWM1);
	timer_enable_oc_output(TIM15, TIM_OC1);
	timer_enable_break_main_output(TIM15);
	timer_set_period(TIM15, LED_TIMER_PERIOD-1);

	timer_enable_counter(TIM15);
	dma_enable_channel(DMA1, DMA_CHANNEL2);



}

//Nastaveni barvy LED podle statusu
void led_dev_set_status(uint8_t perif, uint8_t status){
	uint32_t color;

	switch(status){
		case LED_STATUS_ERR:
			color = LED_CLR_ERROR;
		break;

		case LED_STATUS_OK:
			color = LED_CLR_OK;
		break;

		case LED_STATUS_DATA:
			color = LED_CLR_DATA;
		break;

		case LED_STATUS_LOAD:
			color = LED_CLR_LOAD;
		break;

		default:
			color = 0;

	}

	//Pokude je typu OK, ERR nebo LOAD - nastavi se na pevne sviceni
	if(status == LED_STATUS_OK || status == LED_STATUS_ERR || status == LED_STATUS_LOAD){
		led_statuses[perif] = color;
		led_set_color((perif >> 7) & 0x01, perif & 0x7f, ((led_pending_flags[perif]>>16) & 0xff), ((led_pending_flags[perif]>>8) & 0xff), (led_pending_flags[perif] & 0xff));
	}else if(status == LED_STATUS_DATA){
		//Pokud jde o data, udela se jen probliknuti
		led_pending_flags[perif] = color;
	}
}


//Nastaveni barvy LED podle statusu
void led_dev_set_color(uint8_t perif, uint32_t color){

	led_statuses[perif] = color;
	led_set_color((perif >> 7) & 0x01, perif & 0x7f, ((led_pending_flags[perif]>>16) & 0xff), ((led_pending_flags[perif]>>8) & 0xff), (led_pending_flags[perif] & 0xff));
}


//Rutina pro nastaveni barvy vsech LED najednou
void led_dev_set_color_all(uint8_t strip, uint32_t color){

	//Rozdeli se na dva "pasky" podle nazvu
	for(uint16_t i = 0; i < PERIF_COUNT; i++){
		if(((i & 0x80)>>7) == strip) led_statuses[i] = color;
	}

	//Nastavi se barva
	led_set_strip_color(strip, ((color>>16) & 0xff), ((color>>8) & 0xff), (color & 0xff));

}


//Rutina pro nastaveni barvy vsech LED najednou
void led_dev_set_status_all(uint8_t strip, uint8_t status){
	uint32_t color;

	switch(status){
		case LED_STATUS_ERR:
			color = LED_CLR_ERROR;
		break;

		case LED_STATUS_OK:
			color = LED_CLR_OK;
		break;

		case LED_STATUS_DATA:
			color = LED_CLR_DATA;
		break;

		case LED_STATUS_LOAD:
			color = LED_CLR_LOAD;
		break;

		default:
			color = 0;

	}

	//Rozdeli se na dva "pasky" podle nazvu
	for(uint16_t i = 0; i < PERIF_COUNT; i++){
		if(((i & 0x80)>>7) == strip) led_statuses[i] = color;
	}

	//Nastavi se barva
	led_set_strip_color(strip, ((color>>16) & 0xff), ((color>>8) & 0xff), (color & 0xff));

}



//Rutina pro "probliknuti" kdyz jsou na LED nastavena DATA
void led_dev_process_pending_status(){
	//Projde vsechny LED
	for(uint16_t i = 0; i < PERIF_COUNT; i++){
		if((i & 0x7f) < LED_BACK_NUMBER){
			if(led_pending_flags[i] != 0){
				//Pokud data nebyla nastavena, nastavi barvu a vycisti flag
				led_set_color((i >> 7) & 0x01,i & 0x7f, ((led_pending_flags[i]>>16) & 0xff), ((led_pending_flags[i]>>8) & 0xff), (led_pending_flags[i] & 0xff));
				led_pending_flags[i] = 0;
			}else{
				//Nastavi zpet puvodni barvu led
				led_set_color((i >> 7) & 0x01,i & 0x7f, ((led_statuses[i]>>16) & 0xff), ((led_statuses[i]>>8) & 0xff), (led_statuses[i] & 0xff));
			}
		}
	}

}


void led_set_color(uint8_t strip, uint32_t LEDnumber, uint8_t RED, uint8_t GREEN, uint8_t BLUE) {
	uint8_t tempBuffer[24];
	uint32_t i;
	uint32_t LEDindex;
	if(strip == LED_STRIP_BACK){
		LEDindex = LEDnumber % LED_BACK_NUMBER;
	}else LEDindex = LEDnumber % LED_FRONT_NUMBER;


	for (i = 0; i < 8; i++) // GREEN data
		tempBuffer[i] = ((GREEN << i) & 0x80) ? LED_1 : LED_0;
	for (i = 0; i < 8; i++) // RED
		tempBuffer[8 + i] = ((RED << i) & 0x80) ? LED_1 : LED_0;
	for (i = 0; i < 8; i++) // BLUE
		tempBuffer[16 + i] = ((BLUE << i) & 0x80) ? LED_1 : LED_0;

	if(strip == LED_STRIP_BACK){
		for (i = 0; i < 24; i++)
				ledBackBuffer[LED_RESET_SLOTS_BEGIN + LEDindex * 24 + i] = tempBuffer[i];
	}else{
		for (i = 0; i < 24; i++)
				ledFrontBuffer[LED_RESET_SLOTS_BEGIN + LEDindex * 24 + i] = tempBuffer[i];
	}

}

void led_set_strip_color(uint8_t strip, uint8_t RED, uint8_t GREEN, uint8_t BLUE){
	uint32_t index;

	for (index = 0; index < ((strip == LED_STRIP_FRONT) ? LED_FRONT_NUMBER : LED_BACK_NUMBER); index++)
		led_set_color(strip, index, RED, GREEN, BLUE);
}

void led_fill_buff_black(uint8_t strip){
	uint32_t index, buffIndex;
	buffIndex = 0;

	for (index = 0; index < LED_RESET_SLOTS_BEGIN; index++) {
		if(strip == LED_STRIP_FRONT){
			ledFrontBuffer[buffIndex] = LED_RESET;
		}else ledBackBuffer[buffIndex] = LED_RESET;
		
		buffIndex++;
	}
	for (index = 0; index < ((strip == LED_STRIP_FRONT) ? LED_FRONT_DATA_SIZE : LED_BACK_DATA_SIZE); index++) {
		if(strip == LED_STRIP_FRONT){
			ledFrontBuffer[buffIndex] = LED_0;
		}else ledBackBuffer[buffIndex] = LED_0;
		buffIndex++;
	}
	buffIndex++;
	for (index = 0; index < LED_RESET_SLOTS_END; index++) {
		if(strip == LED_STRIP_FRONT){
			ledFrontBuffer[buffIndex] = 0;
		}else ledBackBuffer[buffIndex] = 0;
		buffIndex++;
	}
}

void led_fill_buff_white(uint8_t strip){
	uint32_t index, buffIndex;
	buffIndex = 0;

	for (index = 0; index < LED_RESET_SLOTS_BEGIN; index++) {
		if(strip == LED_STRIP_FRONT){
			ledFrontBuffer[buffIndex] = LED_RESET;
		}else ledBackBuffer[buffIndex] = LED_RESET;
		
		buffIndex++;
	}
	for (index = 0; index < ((strip == LED_STRIP_FRONT) ? LED_FRONT_DATA_SIZE : LED_BACK_DATA_SIZE); index++) {
		if(strip == LED_STRIP_FRONT){
			ledFrontBuffer[buffIndex] = LED_1;
		}else ledBackBuffer[buffIndex] = LED_1;
		buffIndex++;
	}
	buffIndex++;
	for (index = 0; index < LED_RESET_SLOTS_END; index++) {
		if(strip == LED_STRIP_FRONT){
			ledFrontBuffer[buffIndex] = 0;
		}else ledBackBuffer[buffIndex] = 0;
		buffIndex++;
	}
}
