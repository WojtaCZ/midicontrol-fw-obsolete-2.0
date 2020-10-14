#include <main.h>
#include <setup.h>
#include <led.h>
#include <usb.h>
#include <oled.h>

#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/assert.h>



volatile int cnt = 0;
volatile int tempo = 2000;

void sys_tick_handler(void)
{
	if (cnt++ == tempo) {
		cnt = 0;
		//gpio_toggle(GPIOB, GPIO5);
	}
}

void exti15_10_isr(void)
{
	//gpio_toggle(GPIOB, GPIO5);
	exti_reset_request(EXTI13);
}





int main(void)
{

	setLEDcolor(LED_STRIP_BACK, 0, 255, 0, 0);
	

	clock_setup();
	usb_setup();
	led_setup();
	oled_setup();



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

	nvic_enable_irq(NVIC_EXTI15_10_IRQ);


	

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

	oled_fill(White);

	oled_update();

	while (1) {
		
	}

	return 0;
}

