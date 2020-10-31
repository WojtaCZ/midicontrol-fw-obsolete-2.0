#include "main.h"
#include "setup.h"
#include "led.h"
#include "usb.h"
#include "oled.h"
#include "io.h"
#include "menu.h"
#include "midi.h"
#include "scheduler.h"

#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/iwdg.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/assert.h>
#include <libopencm3/cm3/scb.h>

extern struct Menu menu_main;

void sys_tick_handler(void){

	scheduler_check(&sched_menu_scroll);
	scheduler_check(&sched_menu_update);
	scheduler_check(&sched_led_process);
	scheduler_check(&sched_io_keypress);
	scheduler_check(&sched_oled_sleep);			
}


int main(void)
{

	
	
	int i = 0;

	while(i < 1000000){
		__asm__("nop");
		i++;
	}

	/*#define FW_ADDR    0x1FFF0000

	SCB_VTOR = FW_ADDR & 0xFFFF;

	__asm__ volatile("msr msp, %0"::"g"(*(volatile uint32_t *)FW_ADDR));

	(*(void (**)())(FW_ADDR + 4))();*/

	clock_init();

	io_init();

	oled_init();

	usb_init();

	midi_init();

	//led_init();

	/*led_dev_set_status(LED_DEV_USB, LED_STATUS_OK);
	led_dev_process_pending_status();*/

	menu_show(&menu_main);

	systick_init();

	//rcc_periph_clock_enable(RCC_GPIOB);
	//rcc_periph_clock_enable(RCC_GPIOC);

	/* led on A4 */
	//gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);

	/* button with irq on C13 */
	//rcc_periph_clock_enable(RCC_SYSCFG);

	//gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO13);

	/*exti_select_source(EXTI13, GPIOC);
	exti_set_trigger(EXTI13, EXTI_TRIGGER_RISING);
	exti_enable_request(EXTI13);*/

	//nvic_enable_irq(NVIC_EXTI15_10_IRQ);


	

	/* MCO on A8 */
	//gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);
	//gpio_set_af(GPIOA, GPIO_AF0, GPIO8);
	//gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO8);

	//rcc_set_mcopre(RCC_CFGR_MCOPRE_DIV16);
	//rcc_set_mco(RCC_CFGR_MCO_SYSCLK);

	/* usart1 on pc4/pc5 - arduino tx/rx */
	//rcc_periph_clock_enable(RCC_USART1);
	/*gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO5);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO4);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO5);

	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_CR2_STOPBITS_1);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_enable(USART1);*/
	//usbd_device *usbd_dev;

	/*SYSCFG_CFGR3 |= SYSCFG_CFGR3_ENREF_HSI48 | SYSCFG_CFGR3_EN_VREFINT;
	while (!(SYSCFG_CFGR3 & SYSCFG_CFGR3_REF_HSI48_RDYF));*/

	//led_dev_set_status_all(LED_STRIP_BACK, LED_STATUS_LOAD);
	//led_dev_set_status_all(LED_STRIP_FRONT, LED_STATUS_LOAD);

	while (1) {

		scheduler_process(&sched_menu_scroll);
		scheduler_process(&sched_menu_update);
		//scheduler_process(&sched_led_process);
		scheduler_process(&sched_io_keypress);
		scheduler_process(&sched_oled_sleep);
		
		

	}

	return 0;
}

