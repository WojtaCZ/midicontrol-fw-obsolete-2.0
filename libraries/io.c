#include "io.h"
#include "menu.h"
#include "led.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>


struct Input ioEncoder;

static uint32_t ioSwitchOld = GPIO_ENCODER_SW;
static uint8_t ioPrevNextCode = 0;
static uint16_t ioStore = 0;

extern void oled_wakeup_callback(void);

void io_encoder_read(void){
    static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

    ioPrevNextCode <<= 2;
    if (gpio_get(PORT_ENCODER, GPIO_ENCODER_B)) ioPrevNextCode |= 0x02;
    if (gpio_get(PORT_ENCODER, GPIO_ENCODER_A)) ioPrevNextCode |= 0x01;
    ioPrevNextCode &= 0x0f;

    // If valid then ioStore as 16 bit data.
    if  (rot_enc_table[ioPrevNextCode] ) {
        ioStore <<= 4;
        ioStore |= ioPrevNextCode;
        if ((ioStore&0xff)==0x2b) io_encoder_dec();
        if ((ioStore&0xff)==0x17) io_encoder_inc();
    }

}


void io_sw_read(void){
    uint32_t state = gpio_get(PORT_ENCODER, GPIO_ENCODER_SW);
    if(ioSwitchOld != state){
        ioSwitchOld = state;
        if(state) io_encoder_push();
    }
}

void io_init(void){

    gpio_mode_setup(PORT_ENCODER, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_ENCODER_A | GPIO_ENCODER_B | GPIO_ENCODER_SW);

}

void io_keypress_callback(void){
    
    io_sw_read();
	io_encoder_read();

    if(ioEncoder.inc != 0 || ioEncoder.dec != 0) {
        int pos = ioEncoder.inc - ioEncoder.dec;
        if(pos > 0){
            menu_keypress(MENU_KEY_DOWN);
        }else if(pos < 0){
            menu_keypress(MENU_KEY_UP);
        }
        ioEncoder.inc = 0;
        ioEncoder.dec = 0;
        oled_wakeup_callback();
    }else if(ioEncoder.push != 0){
        menu_keypress(MENU_KEY_ENTER);
        ioEncoder.push = 0;
        oled_wakeup_callback();
    }
}

void io_encoder_dec(void){
    ioEncoder.dec++;
}


void io_encoder_inc(void){
    ioEncoder.inc++;
}


void io_encoder_push(void){
    ioEncoder.push = 1;
}

