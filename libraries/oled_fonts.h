#include <stdint.h>

#ifndef OLED_FONTS_H
#define OLED_FONTS_H

typedef struct {
	const uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;


extern FontDef Font_7x10;
extern FontDef Icon_7x10;
extern FontDef Font_11x18;
extern FontDef Icon_11x18;
extern FontDef Font_16x26;

#endif // __SSD1306_FONTS_H__
