#include <oled.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/dmamux.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>

uint8_t oledInitBuffer[255];
uint8_t oledScreenBuffer[OLED_SCREENBUF_SIZE];
uint8_t oledPageBuffer[4];
uint8_t oledDmaStatus;
uint16_t oledDmaIndex;

static OLED_t oled;

void oled_init(){

    gpio_mode_setup(PORT_I2C1_SCL, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_I2C1_SCL);
    gpio_mode_setup(PORT_I2C1_SDA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_I2C1_SDA);
    gpio_set_output_options(PORT_I2C1_SCL, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, GPIO_I2C1_SCL);
    gpio_set_output_options(PORT_I2C1_SDA, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, GPIO_I2C1_SDA);
	gpio_set_af(PORT_I2C1_SCL, GPIO_AF4, GPIO_I2C1_SCL);
	gpio_set_af(PORT_I2C1_SDA, GPIO_AF4, GPIO_I2C1_SDA);

    dma_set_priority(DMA1, DMA_CHANNEL3, DMA_CCR_PL_HIGH);
	dma_set_memory_size(DMA1, DMA_CHANNEL3, DMA_CCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL3, DMA_CCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL3);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL3);

    dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL3);
    nvic_set_priority(NVIC_DMA1_CHANNEL3_IRQ, 1);
    nvic_enable_irq(NVIC_DMA1_CHANNEL3_IRQ);

	dmamux_set_dma_channel_request(DMAMUX1, DMA_CHANNEL3, DMAMUX_CxCR_DMAREQ_ID_I2C1_TX);

	dma_set_peripheral_address(DMA1, DMA_CHANNEL3, (uint32_t)&I2C1_TXDR);

    i2c_peripheral_disable(I2C1);

    i2c_enable_analog_filter(I2C1);
    i2c_set_digital_filter(I2C1, 0);

	i2c_set_prescaler(I2C1, 5);
	i2c_set_scl_low_period(I2C1, 14);
	i2c_set_scl_high_period(I2C1, 3);
	i2c_set_data_hold_time(I2C1, 1);
	i2c_set_data_setup_time(I2C1, 1);

    i2c_enable_stretching(I2C1);

    i2c_set_7bit_addr_mode(I2C1);
    i2c_peripheral_enable(I2C1);

    uint8_t index = 0;

    //Display Off
    oledInitBuffer[index++] =  OLED_MEM_CMD;
    //Display Off
    oledInitBuffer[index++] = 0xAE;
    //Memory addressing mode
    oledInitBuffer[index++] = 0x20;
    //00,Horizontal Addressing Mode;
    //01,Vertical Addressing Mode;
    //10,Page Addressing Mode (RESET); 
    oledInitBuffer[index++] = 0x10;
    //Set page start address
    oledInitBuffer[index++] = 0xB0;
    //COM Out scan direction
    oledInitBuffer[index++] = 0xC8;
    //Low column addressoled
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

    i2c_transfer7(I2C1, OLED_ADD, oledInitBuffer, index, NULL, 0);

    oled.CurrentX = 0;
    oled.CurrentY = 0;

    oled.Initialized = 1;

}

// Fill the whole screen with the given color
void oled_fill(OLED_COLOR color) {
    /* Set memory */
    uint32_t i;
    for(i = 0; i < sizeof(oledScreenBuffer); i++) {
        if(i % 131){
            oledScreenBuffer[i] = (color == Black) ? 0x00 : 0xFF;
        }else continue;
       
    }
}


//    Draw one pixel in the screenbuffer
//    X => X Coordinate
//    Y => Y Coordinate
//    color => Pixel color
void oled_draw_pixel(uint8_t x, uint8_t y, OLED_COLOR color) {

    x += OLED_XOFFSET;
    y += OLED_YOFFSET;
    
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if(oled.Inverted) {
        color = (OLED_COLOR)!color;
    }

    // Draw in the right color
    if(color == White) {
        oledScreenBuffer[1 + (y/8) + x + (y / 8) * OLED_WIDTH] |= 1 << (y % 8);
    } else {
        oledScreenBuffer[1 + (y/8)+ x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

// Draw 1 char to the screen buffer
// ch         => char om weg te schrijven
// Font     => Font waarmee we gaan schrijven
// color     => Black or White
char oled_write_char(char ch, FontDef Font, OLED_COLOR color) {
    uint32_t i, b, j;

    // Check remaining space on current line
    if (OLED_WIDTH <= (oled.CurrentX + Font.FontWidth) ||
        OLED_HEIGHT <= (oled.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Use the font to write
    for(i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for(j = 0; j < Font.FontWidth; j++) {
            if((b << j) & 0x8000)  {
                oled_draw_pixel(oled.CurrentX + j, (oled.CurrentY + i), (OLED_COLOR) color);
            } else {
                oled_draw_pixel(oled.CurrentX + j, (oled.CurrentY + i), (OLED_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    oled.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

// Write full string to screenbuffer
char oled_write_string(char* str, FontDef Font, OLED_COLOR color) {
    // Write until null-byte
    while (*str) {
        if (oled_write_char(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// Position the cursor
void oled_set_cursor(uint8_t x, uint8_t y) {
    oled.CurrentX = x;
    oled.CurrentY = y;
}

void oled_update(){

    oledPageBuffer[0] = OLED_MEM_CMD;
    oledPageBuffer[1] = 0xB0;
    oledPageBuffer[2] = 0x00;
    oledPageBuffer[3] = 0x10;

    oledScreenBuffer[0] = OLED_MEM_DAT;
    oledScreenBuffer[131] = OLED_MEM_DAT;
    oledScreenBuffer[262] = OLED_MEM_DAT;
    oledScreenBuffer[393] = OLED_MEM_DAT;
    oledScreenBuffer[524] = OLED_MEM_DAT;
    oledScreenBuffer[655] = OLED_MEM_DAT;
    oledScreenBuffer[786] = OLED_MEM_DAT;
    oledScreenBuffer[917] = OLED_MEM_DAT;

    oledDmaStatus = 0;
    oledDmaIndex = 0;

    dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)&oledPageBuffer[0]);
	dma_set_number_of_data(DMA1, DMA_CHANNEL3, 4);
    i2c_set_bytes_to_transfer(I2C1, 4);
    //i2c_enable_interrupt(I2C1, I2C_CR1_TCIE);
    i2c_enable_autoend(I2C1);

    dma_enable_channel(DMA1, DMA_CHANNEL3);

    i2c_enable_txdma(I2C1);
    i2c_send_start(I2C1);

}

void dma1_channel3_isr(void){
    dma_disable_channel(DMA1, DMA_CHANNEL3);
    if(oledDmaStatus < 15){
        if(oledDmaStatus % 2){
            oledPageBuffer[1]++;
            oledDmaIndex += 131;
            dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)&oledPageBuffer[0]);
	        dma_set_number_of_data(DMA1, DMA_CHANNEL3, 4);
            i2c_set_bytes_to_transfer(I2C1, 4);
            dma_enable_channel(DMA1, DMA_CHANNEL3);
        }else{
            dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)&oledScreenBuffer[oledDmaIndex]);
	        dma_set_number_of_data(DMA1, DMA_CHANNEL3, 131);
            i2c_set_bytes_to_transfer(I2C1, 131);
            dma_enable_channel(DMA1, DMA_CHANNEL3);
        }
        oledDmaStatus++;
        i2c_send_start(I2C1);
    }else{
        dma_disable_channel(DMA1, DMA_CHANNEL3);
    }
    dma_clear_interrupt_flags(DMA1, DMA_CHANNEL3, DMA_TCIF);
    nvic_clear_pending_irq(NVIC_DMA1_CHANNEL3_IRQ);
}