#ifndef OLED_H
#define OLED_H

#include <setup.h>
#include <oled_fonts.h>

#define OLED_MEM_CMD			0x00
#define OLED_MEM_DAT			0x40
#define OLED_ADD                0x3C

#define OLED_WIDTH              130
#define OLED_HEIGHT             64
//Velikost bufferu
#define OLED_SCREENBUF_SIZE		(OLED_WIDTH*OLED_HEIGHT / 8) + 8


typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} OLED_COLOR;

// Struct to store transformations
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} OLED_t;


void oled_setup(void);
void oled_update(void);
void oled_fill(OLED_COLOR color);
void oled_drawPixel(uint8_t x, uint8_t y, OLED_COLOR color);
char oled_writeChar(char ch, FontDef Font, OLED_COLOR color);
char oled_writeString(char* str, FontDef Font, OLED_COLOR color);
void oled_setCursor(uint8_t x, uint8_t y);

#endif
