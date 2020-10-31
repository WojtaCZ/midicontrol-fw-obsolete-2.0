#include "setup.h"

#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/assert.h>

void clock_init(void)
{
	const struct rcc_clock_scale rcc_hse_48mhz_3v3[] = {
			{ /* 144MHz */
				.pllm = 6,
				.plln = 36,
				.pllp = 2,
				.pllq = 6,
				.pllr = 2,
				.pll_source = RCC_PLLCFGR_PLLSRC_HSE,
				.hpre = RCC_CFGR_HPRE_NODIV,
				.ppre1 = RCC_CFGR_PPREx_NODIV,
				.ppre2 = RCC_CFGR_PPREx_NODIV,
				.vos_scale = PWR_SCALE1,
				.boost = true,
				.flash_config = FLASH_ACR_DCEN | FLASH_ACR_ICEN,
				.flash_waitstates = 6,
				.ahb_frequency  = 144e6,
				.apb1_frequency = 144e6,
				.apb2_frequency = 144e6,
			}
		};

	rcc_clock_setup_pll(&rcc_hse_48mhz_3v3[0]);

	rcc_wait_for_osc_ready(RCC_HSE);

	pwr_disable_backup_domain_write_protect();
	rcc_osc_on(RCC_LSE);
	rcc_wait_for_osc_ready(RCC_LSE);
	pwr_enable_backup_domain_write_protect();

	
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOE);

	/* Enable clocks for peripherals we need */
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_USART3);
	rcc_periph_clock_enable(RCC_USB);
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_TIM1);
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_clock_enable(RCC_TIM4);
	rcc_periph_clock_enable(RCC_TIM7);
	rcc_periph_clock_enable(RCC_TIM15);
	rcc_periph_clock_enable(RCC_TIM17);
	rcc_periph_clock_enable(RCC_RTCAPB);
	rcc_periph_clock_enable(RCC_SYSCFG);
	rcc_periph_clock_enable(RCC_DMA1);
	rcc_periph_clock_enable(RCC_DMA2);
	//rcc_periph_clock_enable(RCC_PWR);
	//rcc_periph_clock_enable(RCC_WWDG);
	//rcc_periph_clock_enable(RCC_CRC);
	//rcc_periph_clock_enable(RCC_SAI1);
	rcc_periph_clock_enable(RCC_DMAMUX1);

	

}

void systick_init(){
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(143999);
	systick_interrupt_enable();
	systick_clear();
	systick_counter_enable();
}

void wd_init(){
	//iwdg_set_period_ms(10000);
	//iwdg_start();

}

void tim1_init(){

	/*timer_enable_preload(TIM17);
	timer_update_on_overflow(TIM17);
	timer_set_dma_on_update_event(TIM17);
	timer_enable_irq(TIM17, TIM_DIER_CC1DE);
	timer_generate_event(TIM17, TIM_EGR_CC1G);
	timer_set_oc_mode(TIM17, TIM_OC1, TIM_OCM_PWM1);
	timer_enable_oc_output(TIM17, TIM_OC1);
	timer_enable_break_main_output(TIM17);
	timer_set_period(TIM17, LED_TIMER_PERIOD-1);

	timer_enable_counter(TIM17);*/




	nvic_enable_irq(NVIC_TIM2_IRQ);

	/* Reset TIM2 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM2);

	/* Timer global mode:
	 * - No divider
	 * - Alignment edge
	 * - Direction up
	 * (These are actually default values after reset above, so this call
	 * is strictly unnecessary, but demos the api for alternative settings)
	 */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*
	 * Please take note that the clock source for STM32 timers
	 * might not be the raw APB1/APB2 clocks.  In various conditions they
	 * are doubled.  See the Reference Manual for full details!
	 * In our case, TIM2 on APB1 is running at double frequency, so this
	 * sets the prescaler to have the timer run at 5kHz
	 */
	timer_set_prescaler(TIM2, ((rcc_apb1_frequency * 2) / 5000));

	/* Disable preload. */
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);

	/* count full range, as we'll update compare value continuously */
	timer_set_period(TIM2, 65535);

	/* Set the initual output compare value for OC1. */
	//timer_set_oc_value(TIM2, TIM_OC1, frequency_sequence[frequency_sel++]);

	/* Counter enable. */
	timer_enable_counter(TIM2);

	/* Enable Channel 1 compare interrupt to recalculate compare values */
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}