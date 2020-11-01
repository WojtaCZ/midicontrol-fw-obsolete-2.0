#ifndef MENU_H
#define MENU_H

#include "oled.h"
#include "oled_fonts.h"

#define MENU_LANGUAGES 2

// If you use different languages, define them here
#define LANGUAGE_CZECH 0
#define LANGUAGE_ENGLISH 1

// Set rows/cols based on your font (for graphical displays)
#define ROW(x) ((x)*18)
#define COL(y) ((y)*11)

#define MENU_TOP_OFFSET			21
#define MENU_LEFT_OFFSET		0
#define MENU_TEXT_SPACING		3

// Number of items on one screen
// Not including title
#define MENU_LINES 2

// Symbol which is displayed in front of the selected item
// This symbol doesn't appear when MENU_LINES == 1
#define MENU_SELECTED_SYMBOL								32	
#define MENU_NOT_SELECTED_SYMBOL 							33

#define MENU_SELECTED_CHECKBOX_CHECKED_SYMBOL 				41
#define MENU_NOT_SELECTED_CHECKBOX_CHECKED_SYMBOL 			39
#define MENU_SELECTED_CHECKBOX_NOT_CHECKED_SYMBOL 			40
#define MENU_NOT_SELECTED_CHECKBOX_NOT_CHECKED_SYMBOL 		38

#define MENU_ICON_FONT Icon_11x18

#define MENU_SCROLL_PAUSE	2

// How many spaces is between arrow symbol and menu item
// useful to set zero on smaller displays
#define MENU_SELECTOR_SPACING	 3

// 0-3 bits
#define MENU_PARAMETER_MASK 0x0F
#define MENU_PARAMETER_IS_NUMBER 1
#define MENU_PARAMETER_IS_STRING 2

// 4. bit checkbox bit
#define MENU_ITEM_IS_CHECKBOX	0x10
// 5bit
#define MENU_ITEM_IS_CHECKED	0x20

// 6.bit - submenu bit
#define MENU_CALLBACK_IS_SUBMENU	0x40

// 7bit - callback bit
#define MENU_CALLBACK_IS_FUNCTION 0x80


#define MENU_KEY_UP		10	
#define MENU_KEY_DOWN	20
#define MENU_KEY_ENTER	30

#define MENU_FONT		Font_11x18

typedef struct SMenuItem {
	char* text[MENU_LANGUAGES];
	void (*callback)(void *);
	int flags;
	int parameter;
	uint8_t specSelected;
	uint8_t specNotSelected;
} MenuItem;


typedef struct SMenu {
	char* title[MENU_LANGUAGES];
	int selectedIndex;
	void (*parent)(void *);
	MenuItem *items[];
} Menu;


void menu_show(Menu *menu);
void menu_update(void);
void menu_keypress(uint8_t key);
void menu_back(void);
void menu_scroll_callback(void);


#endif