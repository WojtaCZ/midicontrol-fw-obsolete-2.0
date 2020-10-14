#include <setup.h>

#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/assert.h>

void clock_setup(void)
{
	const struct rcc_clock_scale rcc_hse_48mhz_3v3[RCC_CLOCK_3V3_END] = {
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
				.flash_waitstates = 4,
				.ahb_frequency  = 144e6,
				.apb1_frequency = 144e6,
				.apb2_frequency = 144e6,
			}
		};

	rcc_clock_setup_pll(&rcc_hse_48mhz_3v3[0]);

	// Systick na 1khz
	systick_set_frequency(1000, rcc_ahb_frequency);
	systick_interrupt_enable();
	systick_counter_enable();

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
	rcc_periph_clock_enable(RCC_DMAMUX1);


}
