#ifndef OLED_H
#define OLED_H

#include "global.h"
#include "oled_fonts.h"

#define OLED_MEM_CMD			0x00
#define OLED_MEM_DAT			0x40
#define OLED_ADD                0x3C

#define OLED_WIDTH              130
#define OLED_HEIGHT             64

#define OLED_XOFFSET            2
#define OLED_YOFFSET            0

#define OLED_SLEEP_INTERVAL     30000

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
    uint8_t Sleep;
} OLED_t;


void oled_init(void);
void oled_update(void);
void oled_fill(OLED_COLOR color);
void oled_draw_pixel(uint8_t x, uint8_t y, OLED_COLOR color);
char oled_write_char(char ch, FontDef Font, OLED_COLOR color);
char oled_write_string(char* str, FontDef Font, OLED_COLOR color);
void oled_set_cursor(uint8_t x, uint8_t y);

void oled_sleep_callback(void);
void oled_wakeup_callback(void);

#endif
