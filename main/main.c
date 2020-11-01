#include "main.h"
#include "../libraries/setup.h"
#include "../libraries/led.h"
#include "../libraries/usb.h"
#include "../libraries/oled.h"
#include "../libraries/io.h"
#include "../libraries/menu.h"
#include "../libraries/midi.h"
#include "../libraries/scheduler.h"

#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/iwdg.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/assert.h>


extern Menu menu_main;

void sys_tick_handler(void){

	scheduler_check(&sched_menu_scroll);
	scheduler_check(&sched_menu_update);
	scheduler_check(&sched_led_process);
	scheduler_check(&sched_io_keypress);
	scheduler_check(&sched_oled_sleep);	
	scheduler_check(&sched_comm_decode);		
}


int main(void)
{

	
	//Pauza pro startup (zmizi zakmity na PSU)
	int i = 0;
	while(i < 1000000){
		__asm__("nop");
		i++;
	}


	clock_init();

	io_init();

	//bootloader_check();

	oled_init();

	usb_init();

	midi_init();

	//led_init();

	/*led_dev_set_status(LED_DEV_USB, LED_STATUS_OK);
	led_dev_process_pending_status();*/

	menu_show(&menu_main);

	systick_init();



	//led_dev_set_status_all(LED_STRIP_BACK, LED_STATUS_LOAD);
	//led_dev_set_status_all(LED_STRIP_FRONT, LED_STATUS_LOAD);

	while (1) {
		scheduler_process(&sched_menu_scroll);
		scheduler_process(&sched_menu_update);
		//scheduler_process(&sched_led_process);
		scheduler_process(&sched_io_keypress);
		scheduler_process(&sched_oled_sleep);
		scheduler_process(&sched_comm_decode);

	}

	return 0;
}

