#include <oled.h>
#include <setup.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/dmamux.h>
#include <libopencm3/stm32/i2c.h>

uint8_t oledInitBuffer[];

void oled_setup(){
    uint8_t index = 0;

    //Display Off
    oledInitBuffer[index++] = 0xAE;
    //Memory addressing mode
    oledInitBuffer[index++] = 0x10;
    //00,Horizontal Addressing Mode;
    //01,Vertical Addressing Mode;
    //10,Page Addressing Mode (RESET); 
    oledInitBuffer[index++] = 0x10;
    //Set page start address
    oledInitBuffer[index++] = 0xB0;
    //COM Out scan direction
    oledInitBuffer[index++] = 0xC8;
    //Low column address
    oledInitBuffer[index++] = 0x00;
    //High column address
    oledInitBuffer[index++] = 0x10;
    //Start line address
    oledInitBuffer[index++] = 0x40;
    //Contrast
    oledInitBuffer[index++] = 0x81;
    oledInitBuffer[index++] = 0xFF;
    //Segment remap 0 to 127
    oledInitBuffer[index++] = 0xA1;
    //Normal color
    oledInitBuffer[index++] = 0xA6;
    //Set MUX ratio
    oledInitBuffer[index++] = 0xA8;
    oledInitBuffer[index++] = 0x3F;
    //Output follows RAM
    oledInitBuffer[index++] = 0xA4;
    //Set display offset
    oledInitBuffer[index++] = 0xD3;
    oledInitBuffer[index++] = 0x00;
    //Set display clock divide ratio
    oledInitBuffer[index++] = 0xD5;
    oledInitBuffer[index++] = 0xF0;
    //Set precharge period
    oledInitBuffer[index++] = 0xD9;
    oledInitBuffer[index++] = 0x34;
    //Set COM pin HW config
    oledInitBuffer[index++] = 0xDA;
    oledInitBuffer[index++] = 0x12;
    //Set VCOMH (0.77*Vcc)
    oledInitBuffer[index++] = 0xDB;
    oledInitBuffer[index++] = 0x20;
    //Set DC-DC enable
    oledInitBuffer[index++] = 0x8D;
    oledInitBuffer[index++] = 0x14;
    //Turn on oled
    oledInitBuffer[index++] = 0xAF;

    



}


