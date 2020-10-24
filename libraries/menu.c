#include "menu.h"
#include "oled.h"
#include "scheduler.h"
#include <string.h>


/*MenuItem item0 = {{"Tick"}, callback, MENU_PARAMETER_IS_NUMBER | MENU_CALLBACK_IS_FUNCTION, (int)&msTick};
MenuItem item1 = {{"Lang SubMnu"}, (void*)&subMenu,  MENU_CALLBACK_IS_SUBMENU, 0};
MenuItem itemTime = {{"Time"}, callback, MENU_PARAMETER_IS_STRING | MENU_CALLBACK_IS_FUNCTION, (int)"12:05"};
MenuItem item2 = {{"Count"}, callback, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED | MENU_CALLBACK_IS_FUNCTION};
MenuItem item3 = {{"Lights"}, callback, MENU_ITEM_IS_CHECKBOX  };
MenuItem item4 = {{"Sound"}, callback, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED };
MenuItem item5 = {{"Fan"}, callback, MENU_ITEM_IS_CHECKBOX };
MenuItem item6 = {{"Buzzer"}, callback, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED };
MenuItem item7 = {{"Last check"}, callback, MENU_ITEM_IS_CHECKBOX };*/

extern Scheduler sched_menu_scroll;
extern Scheduler sched_menu_update;

Menu * menu_actual;

extern Menu menu_main, menu_display, menu_settings;

void callback(){

}




MenuItem item2 = {{"Count"}, callback, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED | MENU_CALLBACK_IS_FUNCTION};
MenuItem item3 = {{"Lights"}, callback, MENU_CALLBACK_IS_FUNCTION};
MenuItem item4 = {{"Sound"}, callback, MENU_ITEM_IS_CHECKBOX | MENU_ITEM_IS_CHECKED };
MenuItem item5 = {{"Fan"}, callback, MENU_ITEM_IS_CHECKBOX };



MenuItem menuitem_play = {{"Prehraj", "Play"}, callback, MENU_CALLBACK_IS_FUNCTION};
MenuItem menuitem_record = {{"Nahraj", "Record"}, callback, MENU_CALLBACK_IS_FUNCTION};
MenuItem menuitem_organ_pwr = {{"Napajeni varhan", "Organ power"}, callback, MENU_CALLBACK_IS_FUNCTION | MENU_ITEM_IS_CHECKBOX};
MenuItem menuitem_display = {{"Ukazatel", "Display"}, (void*)&menu_display, MENU_CALLBACK_IS_SUBMENU};
MenuItem menuitem_settings = {{"Nastaveni", "Settings"}, (void*)&menu_settings, MENU_CALLBACK_IS_SUBMENU};

MenuItem menuitem_back = {{"Zpet", "Back"}, (void*)&menu_back, MENU_CALLBACK_IS_FUNCTION, .specSelected = 37, .specNotSelected = 36};



Menu menu_main = {
	{"Hlavni menu","Main menu"},
    .items = {&menuitem_play, &menuitem_record, &menuitem_organ_pwr, &menuitem_display, &menuitem_settings, 0},
};

Menu menu_display = {
    {"Ukazatel","Display"},
	.parent = &menu_main,
    .items = { &item2, &item3, &item4, &item5, &menuitem_back, 0},
};

Menu menu_settings = {
    {"Nastaveni","Settings"},
	.parent = &menu_main,
    .items = { &item2, &item3, &item4, &item5, &menuitem_back, 0},
};





unsigned char menuLanguage;

int i = 0;

unsigned char menuItem = 0;
unsigned char lastMenuItem = 255;

unsigned char cursorTopPos = 0;
unsigned char menuTopPos = 0;

int len = 0;

uint8_t menu_scrollIndex = 0, menu_scrollPause = 0, menu_scrollPauseDone = 0, menu_scrollMax;

void menu_scroll_callback(){
	//Menu text scrolling
	if(menu_scrollPauseDone){
		if(menu_scrollIndex <= menu_scrollMax){
			menu_scrollIndex++;
		}else{
			menu_scrollPauseDone = 0;
		}
	}else menu_scrollPause++;

	if(menu_scrollPause == MENU_SCROLL_PAUSE){
		if(menu_scrollIndex > 0){
			menu_scrollPauseDone = 0;
		}else menu_scrollPauseDone = 1;

		menu_scrollPause = 0;
		menu_scrollIndex = 0;
	}

}

void menu_show(Menu *menu){

	//Setup pointer to shown menu (for other functions)
	menu_actual = menu;

	//Zero out some variables
	len = 0;
	cursorTopPos = 0;
	menuTopPos = 0;

	//Count menu items
	MenuItem **iList = menu_actual->items;
	for (; *iList != 0; ++iList){
		len++;
	}
	  
	if(menu_actual->selectedIndex != -1){
	  // If the item is on the first screen
	  if(menu_actual->selectedIndex < MENU_LINES){
		  menuItem = menu_actual->selectedIndex;
		  cursorTopPos = menu_actual->selectedIndex;
		  menuTopPos = 0;
	  }else{
		 //If the item is on the other screen
		  menuItem = menu_actual->selectedIndex;
		  cursorTopPos = MENU_LINES - 1;
		  menuTopPos = menu_actual->selectedIndex - cursorTopPos;
	  }
	}

}



void menu_keypress(uint8_t key){

	//Get actual menu from pointer
	Menu * menu = menu_actual;

	//Move down event
	if(key == MENU_KEY_DOWN){
		if(menuItem != len-1){
			menuItem++;

			if(cursorTopPos >= MENU_LINES-1 || (cursorTopPos ==((MENU_LINES)/2) && ((len) - menuItem  ) > ((MENU_LINES-1)/2))){
				menuTopPos++;
			}else cursorTopPos++;

		}else{
			menuItem = 0;
			cursorTopPos = 0;
			menuTopPos = 0;
		}
	}

	//Move up event
	if(key == MENU_KEY_UP){
		if(menuItem != 0){
			menuItem--;

			if(cursorTopPos > 0 && !((cursorTopPos == MENU_LINES/2) && (menuItem >= MENU_LINES/2))){
				cursorTopPos--;
			}else menuTopPos--;
		}else{

			menuItem = len-1;

			if(len <= MENU_LINES){
				menuTopPos = 0;
			}else{
				menuTopPos = menuItem;
			}
			
			if(menuTopPos > len - MENU_LINES && len >= MENU_LINES){
				menuTopPos = len - MENU_LINES;
			}

			cursorTopPos = menuItem - menuTopPos;
		}
	}

	//Enter event
	if(key == MENU_KEY_ENTER){
		key = 0;
		menu->selectedIndex = menuItem;
		int flags = menu->items[menu->selectedIndex]->flags;

		//Checkbox without function callback
		if((menu->items[menu->selectedIndex]->callback == 0) && (flags & MENU_ITEM_IS_CHECKBOX)){
			menu->items[menu->selectedIndex]->flags ^= MENU_ITEM_IS_CHECKED;
			lastMenuItem = -1;
		}

		//Menu callback
		if(flags & MENU_CALLBACK_IS_SUBMENU && menu->items[menu->selectedIndex]->callback){
			menu_show((Menu*)menu->items[menu->selectedIndex]->callback);

			lastMenuItem = -1;
		}

		//Function callback
		if(flags & MENU_CALLBACK_IS_FUNCTION && menu->items[menu->selectedIndex]->callback){
			(*menu->items[menu->selectedIndex]->callback)(menu);

			lastMenuItem = -1;
		}

	}
}

//Callback for back item click
void menu_back(){
	menu_actual->selectedIndex = 0;
	menu_show((Menu*)menu_actual->parent);			
}

void menu_update(){

	Menu * menu = menu_actual;

	if(lastMenuItem != menuItem){

		
		oled_fill(Black);
		
		if(menu->parent != 0){
			oled_set_cursor(0,0);
			oled_write_string(menu->title[menuLanguage], Font_7x10, White);
		}

		if(menuItem == len-1){
			//Down arrow
			oled_set_cursor(117,19);
			oled_write_char(34, Icon_11x18, White);
		}else if(menuItem <= 0){
			//Up arrow
			oled_set_cursor(117,41);
			oled_write_char(35, Icon_11x18, White);
		}else if(menuItem > 0 && menuItem < len-1){
			//Down arrow
			oled_set_cursor(117,19);
			oled_write_char(34, Icon_11x18, White);
			//Up arrow
			oled_set_cursor(117,41);
			oled_write_char(35, Icon_11x18, White);
		}

		  // Menu debug
		  char buffer [64];
		  sprintf(buffer, "%d,%d,%d", menuItem, lastMenuItem, menuTopPos);
		  oled_set_cursor(0,0);
		  oled_write_string(buffer, Font_7x10, White);


		if(strlen(menu->items[menuItem]->text[menuLanguage]) > 8 /*&& !(sched_menu_scroll.flags & SCHEDULER_ON)*/){
			menu_scrollIndex = 0;
			menu_scrollMax = strlen(menu->items[menuItem]->text[menuLanguage])-9;
			menu_scrollPause = 0;
			menu_scrollPauseDone = 0;

			sched_menu_scroll.flags |= SCHEDULER_ON;				
		}else sched_menu_scroll.flags &= ~SCHEDULER_ON;

		i = 0;
		while((i + menuTopPos) < len && i < MENU_LINES){
			int index = menuTopPos + i;
			if(menuItem == index && MENU_LINES > 1){
				oled_set_cursor(0 + MENU_LEFT_OFFSET, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
				if(menu->items[index]->specSelected != 0 && !(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX)){
					oled_write_char(menu->items[index]->specSelected, MENU_ICON_FONT, White);
				}else if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX){
					if(menu->items[index]->flags & MENU_ITEM_IS_CHECKED){
						oled_write_char(MENU_SELECTED_CHECKBOX_CHECKED_SYMBOL, MENU_ICON_FONT, White);
					}else{
						oled_write_char(MENU_SELECTED_CHECKBOX_NOT_CHECKED_SYMBOL, MENU_ICON_FONT, White);
					}
				}else{
					oled_write_char(MENU_SELECTED_SYMBOL, MENU_ICON_FONT, White);
				}
			}else{
				oled_set_cursor(0 + MENU_LEFT_OFFSET, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
				if(menu->items[index]->specNotSelected != 0 && !(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX)){
					oled_write_char(menu->items[index]->specNotSelected, MENU_ICON_FONT, White);
				}else if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX){
					if(menu->items[index]->flags & MENU_ITEM_IS_CHECKED){
						oled_write_char(MENU_NOT_SELECTED_CHECKBOX_CHECKED_SYMBOL, MENU_ICON_FONT, White);
					}else{
						oled_write_char(MENU_NOT_SELECTED_CHECKBOX_NOT_CHECKED_SYMBOL, MENU_ICON_FONT, White);
					}
				}else{
					oled_write_char(MENU_NOT_SELECTED_SYMBOL, MENU_ICON_FONT, White);
				}
				
			}

			int posx = strlen(menu->items[index]->text[menuLanguage]) + 3;



			if(MENU_LINES > 1){
				if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX){
					oled_set_cursor(COL(1) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
				}else oled_set_cursor(COL(1) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
				
				if(strlen(menu->items[index]->text[menuLanguage]) > 8){
					char tmp[9];
					if(menuItem == index){
						memcpy(tmp, menu->items[index]->text[menuLanguage]+menu_scrollIndex, 8);
						memset(tmp+8, 0, strlen(menu->items[index]->text[menuLanguage])-8);
					}else{
						memcpy(tmp, menu->items[index]->text[menuLanguage], 8);
						memset(tmp+8, 0, strlen(menu->items[index]->text[menuLanguage])-8);
					}

					oled_write_string(tmp, MENU_FONT, White);
				}else oled_write_string(menu->items[index]->text[menuLanguage], MENU_FONT, White);

			}else{
				if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX){
					oled_set_cursor(COL(0) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
				}else oled_set_cursor(COL(0) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);

				if(strlen(menu->items[index]->text[menuLanguage]) > 8){
					char tmp[9];
					if(menuItem == index){
						memcpy(tmp, menu->items[index]->text[menuLanguage]+menu_scrollIndex, 8);
						memset(tmp+8, 0, strlen(menu->items[index]->text[menuLanguage])-8);
					}else{
						memcpy(tmp, menu->items[index]->text[menuLanguage], 8);
						memset(tmp+8, 0, strlen(menu->items[index]->text[menuLanguage])-8);
					}

					oled_write_string(tmp, MENU_FONT, White);
				}else oled_write_string(menu->items[index]->text[menuLanguage], MENU_FONT, White);
			}
				if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_NUMBER){
					oled_set_cursor(COL(posx) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
					oled_write_string(*((int*)menu->items[index]->parameter), MENU_FONT, White);
				}

				if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_STRING){
					oled_set_cursor(COL(posx) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
					oled_write_string((char*)(menu->items[index]->parameter), MENU_FONT, White);
				}

			i++;
		}
		  lastMenuItem = menuItem;
	}

	oled_update();

}