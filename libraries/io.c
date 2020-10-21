#include <io.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>

static uint32_t switchOld = 0;
static uint8_t prevNextCode = 0;
static uint16_t store=0;

void io_encoder_read(void){
    static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

    prevNextCode <<= 2;
    if (gpio_get(PORT_ENCODER, GPIO_ENCODER_B)) prevNextCode |= 0x02;
    if (gpio_get(PORT_ENCODER, GPIO_ENCODER_A)) prevNextCode |= 0x01;
    prevNextCode &= 0x0f;

    // If valid then store as 16 bit data.
    if  (rot_enc_table[prevNextCode] ) {
        store <<= 4;
        store |= prevNextCode;
        if ((store&0xff)==0x2b) io_encoder_dec();
        if ((store&0xff)==0x17) io_encoder_inc();
    }

}


void io_sw_read(void){
    uint32_t state = gpio_get(PORT_ENCODER, GPIO_ENCODER_SW);
    if(switchOld != state){
        switchOld = state;
        if(state) io_encoder_push();
    }
}

void io_init(void){

    gpio_mode_setup(PORT_ENCODER, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_ENCODER_A | GPIO_ENCODER_B | GPIO_ENCODER_SW);

}


void io_encoder_dec(void){

}


void io_encoder_inc(void){
    
}


void io_encoder_push(void){
    
}