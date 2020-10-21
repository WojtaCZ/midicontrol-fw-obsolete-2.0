/*
 *
 *
 * Menu2 v2.0
 * Martin Hubacek
 * 18.3.2013
 * http://martinhubacek.cz
 *
 *
 */


#ifndef MENU_H
#define MENU_H

#include "oled.h"

#define MENU_LANGUAGES 2

// If you use different languages, define them here
#define LANGUAGE_CZECH 0
#define LANGUAGE_ENGLISH 1

unsigned char menuLanguage;

// Set rows/cols based on your font (for graphical displays)
#define ROW(x) ((x)*64)
#define COL(y) ((y)*128)
// For character LCDs use definitions below
//#define ROW(x) (x)
//#define COL(y) (y)

// Number of items on one screen
// Not including title
#define MENU_LINES 5

//1
//2-scroll dolu - sipka neni na 1. radku
//3ok
//4 posledni scroll dolu blbne
//5ok

// Symbol which is displayed in front of the selected item
// This symbol doesn't appear when MENU_LINES == 1
#define ARROW_SYMBOL ">"
// How many spaces is between arrow symbol and menu item
// useful to set zero on smaller displays
#define ARROW_GAP 1

// Clear display
#define displayClear()	oled_fill(Black);


// Optional function to write buffer to display - comment if not used
#define displayDraw()		oled_update();


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



typedef struct SMenuItem {
	char* text[MENU_LANGUAGES];
	void (*callback)(void *);
	int flags;
	int parameter;
} MenuItem;


typedef struct SMenu {
	char* title[MENU_LANGUAGES];
	int selectedIndex;
	int refresh;
	MenuItem *items[];
} Menu;



int menu2(Menu *menu);

#endif