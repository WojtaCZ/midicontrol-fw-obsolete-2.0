#include "menu.h"
#include "oled.h"
#include "scheduler.h"

#include <string.h>


unsigned char menuLanguage;

unsigned char menuItem = 0;
unsigned char menuLastMenuItem = 255;

unsigned char menuCursorTopPos = 0;
unsigned char menuTopPos = 0;

int len = 0, i = 0;

uint8_t menuScrollIndex = 0, menuScrollPause = 0, menuScrollPauseDone = 0, menuScrollMax, menuForceUpdate = 0, menuMode = MENU_MENUMODE_NORMAL;

extern Scheduler sched_menu_scroll;
extern Scheduler sched_menu_update;

Menu * menu_actual_menu;
void (*menu_actual_splash)(void *);
void * menu_actual_splash_param;
uint8_t menuActualSplashKeypress = 0;

extern Menu menu_main, menu_display, menu_settings, menu_set_display;

extern void base_menu_set_current_source(void * m);
extern void base_play(uint8_t initiator, char * songname);
extern void base_stop(uint8_t initiator);
extern void base_req_songlist();

MenuItem menuitem_play = {{"Prehraj", "Play"}, (void*)&base_req_songlist, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_stop = {{"Zastav", "Stop"}, (void*)&base_stop, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_record = {{"Nahraj", "Record"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_organ_pwr = {{"Napajeni varhan", "Organ power"}, (void*)&base_menu_set_current_source, MENU_CALLBACK_IS_FUNCTION | MENU_ITEM_IS_CHECKBOX, 0, 0, 0};
MenuItem menuitem_display = {{"Ukazatel", "Display"}, (void*)&menu_display, MENU_CALLBACK_IS_SUBMENU, 0, 0, 0};
MenuItem menuitem_settings = {{"Nastaveni", "Settings"}, (void*)&menu_settings, MENU_CALLBACK_IS_SUBMENU, 0, 0, 0};

MenuItem menuitem_back = {{"Zpet", "Back"}, (void*)&menu_back, MENU_CALLBACK_IS_FUNCTION, 0, 37, 36};

MenuItem menuitem_paired_devices = {{"Sparovana zarizeni", "Paired devices"}, 0, MENU_CALLBACK_IS_SUBMENU, 0, 0, 0};
MenuItem menuitem_midi_loopback = {{"MIDI Loopback", "MIDI Loopback"}, 0, MENU_CALLBACK_IS_FUNCTION | MENU_ITEM_IS_CHECKBOX, 0, 0, 0};
MenuItem menuitem_midi_state = {{"MIDI Stav", "MIDI state"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_device_info = {{"Informace o zarizeni", "Device info"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};

MenuItem menuitem_display_set = {{"Nastavit", "Set"}, (void*)&menu_set_display, MENU_CALLBACK_IS_SUBMENU, 0, 0, 0};
MenuItem menuitem_display_state = {{"Stav", "State"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};

MenuItem menuitem_display_set_song = {{"Pisen", "Song"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_display_set_verse = {{"Sloku", "Verse"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_display_set_led = {{"LED", "LED"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};
MenuItem menuitem_display_set_letter = {{"Pismeno", "Letter"}, 0, MENU_CALLBACK_IS_FUNCTION, 0, 0, 0};


Menu menu_main = {
	{"Hlavni menu","Main menu"},
    .items = {&menuitem_play, &menuitem_record, &menuitem_organ_pwr, &menuitem_display, &menuitem_settings, 0},
};

Menu menu_playing = {
	{"Prehrava se","Now playing"},
    .items = {&menuitem_stop, 0},
};


Menu menu_settings = {
    {"Nastaveni","Settings"},
	.parent = &menu_main,
    .items = { &menuitem_paired_devices, &menuitem_midi_loopback, &menuitem_midi_state, &menuitem_device_info, &menuitem_back, 0},
};

Menu menu_display = {
    {"Ukazatel","Display"},
	.parent = &menu_main,
    .items = { &menuitem_display_state, &menuitem_display_set, &menuitem_back, 0},
};

Menu menu_set_display = {
    {"Nastavit ukazatel","Set display"},
	.parent = &menu_display,
    .items = { &menuitem_display_set_song, &menuitem_display_set_verse, &menuitem_display_set_led, &menuitem_display_set_letter, &menuitem_back, 0},
};

void menu_scroll_callback(void){
	//Menu text scrolling
	if(menuScrollPauseDone){
		if(menuScrollIndex <= menuScrollMax){
			menuScrollIndex++;
		}else{
			menuScrollPauseDone = 0;
		}
	}else menuScrollPause++;

	if(menuScrollPause == MENU_SCROLL_PAUSE){
		if(menuScrollIndex > 0){
			menuScrollPauseDone = 0;
		}else menuScrollPauseDone = 1;

		menuScrollPause = 0;
		menuScrollIndex = 0;
	}

	menuForceUpdate = 1;

}

void menu_show(Menu *menu){

	//Setup pointer to shown menu (for other functions)
	menu_actual_menu = menu;

	//Zero out some variables
	len = 0;
	menuCursorTopPos = 0;
	menuTopPos = 0;

	//Count menu items
	MenuItem **iList = menu_actual_menu->items;
	for (; *iList != 0; ++iList){
		len++;
	}
	  
	if(menu_actual_menu->selectedIndex != -1){
	  // If the item is on the first screen
	  if(menu_actual_menu->selectedIndex < MENU_LINES){
		  menuItem = menu_actual_menu->selectedIndex;
		  menuCursorTopPos = menu_actual_menu->selectedIndex;
		  menuTopPos = 0;
	  }else{
		 //If the item is on the other screen
		  menuItem = menu_actual_menu->selectedIndex;
		  menuCursorTopPos = MENU_LINES - 1;
		  menuTopPos = menu_actual_menu->selectedIndex - menuCursorTopPos;
	  }
	}

}



void menu_keypress(uint8_t key){

	if(menuMode == MENU_MENUMODE_NORMAL){
		//Get actual menu from pointer
		Menu * menu = menu_actual_menu;

		//Move down event
		if(key == MENU_KEY_DOWN){
			if(menuItem != len-1){
				menuItem++;

				if(menuCursorTopPos >= MENU_LINES-1 || (menuCursorTopPos ==((MENU_LINES)/2) && ((len) - menuItem  ) > ((MENU_LINES-1)/2))){
					menuTopPos++;
				}else menuCursorTopPos++;

			}else{
				menuItem = 0;
				menuCursorTopPos = 0;
				menuTopPos = 0;
			}
		}

		//Move up event
		if(key == MENU_KEY_UP){
			if(menuItem != 0){
				menuItem--;

				if(menuCursorTopPos > 0 && !((menuCursorTopPos == MENU_LINES/2) && (menuItem >= MENU_LINES/2))){
					menuCursorTopPos--;
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

				menuCursorTopPos = menuItem - menuTopPos;
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
				menuLastMenuItem = -1;
			}

			//Menu callback
			if(flags & MENU_CALLBACK_IS_SUBMENU && menu->items[menu->selectedIndex]->callback){
				menu_show((Menu*)menu->items[menu->selectedIndex]->callback);

				menuLastMenuItem = -1;
			}

			//Function callback
			if(flags & MENU_CALLBACK_IS_FUNCTION && menu->items[menu->selectedIndex]->callback){
				(*menu->items[menu->selectedIndex]->callback)(menu);

				menuLastMenuItem = -1;
			}

		}
	}else if(menuMode == MENU_MENUMODE_SPLASH){
		menuActualSplashKeypress = key;
	}
}

//Callback for back item click
void menu_back(void){
	menu_actual_menu->selectedIndex = 0;
	menu_show((Menu*)menu_actual_menu->parent);			
}

void menu_update(void){

	Menu * menu = menu_actual_menu;

	if((menuLastMenuItem != menuItem || menuForceUpdate) && menuMode == MENU_MENUMODE_NORMAL){

		menuForceUpdate = 0;
		
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
		  


		if(strlen(menu->items[menuItem]->text[menuLanguage]) > 9 && !(sched_menu_scroll.flags & SCHEDULER_ON)){
			menuScrollIndex = 0;
			menuScrollMax = strlen(menu->items[menuItem]->text[menuLanguage])-10;
			menuScrollPause = 0;
			menuScrollPauseDone = 0;

			sched_menu_scroll.flags |= SCHEDULER_ON;				
		}else if(strlen(menu->items[menuItem]->text[menuLanguage]) <= 8) sched_menu_scroll.flags &= ~SCHEDULER_ON;

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
				
				if(strlen(menu->items[index]->text[menuLanguage]) > 9){
					char tmp[10];
					if(menuItem == index){
						memcpy(tmp, menu->items[index]->text[menuLanguage]+menuScrollIndex, 9);
						memset(tmp+9, 0, strlen(menu->items[index]->text[menuLanguage])-9);
					}else{
						memcpy(tmp, menu->items[index]->text[menuLanguage], 9);
						memset(tmp+9, 0, strlen(menu->items[index]->text[menuLanguage])-9);
					}

					oled_write_string(tmp, MENU_FONT, White);
				}else oled_write_string(menu->items[index]->text[menuLanguage], MENU_FONT, White);

			}else{
				if(menu->items[index]->flags & MENU_ITEM_IS_CHECKBOX){
					oled_set_cursor(COL(0) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
				}else oled_set_cursor(COL(0) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);

				if(strlen(menu->items[index]->text[menuLanguage]) > 9){
					char tmp[10];
					if(menuItem == index){
						memcpy(tmp, menu->items[index]->text[menuLanguage]+menuScrollIndex, 9);
						memset(tmp+9, 0, strlen(menu->items[index]->text[menuLanguage])-9);
					}else{
						memcpy(tmp, menu->items[index]->text[menuLanguage], 9);
						memset(tmp+9, 0, strlen(menu->items[index]->text[menuLanguage])-9);
					}

					oled_write_string(tmp, MENU_FONT, White);
				}else oled_write_string(menu->items[index]->text[menuLanguage], MENU_FONT, White);
			}
				if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_NUMBER){
					oled_set_cursor(COL(posx) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
					char buff[64];
					sprintf(buff, "%d", menu->items[index]->parameter);
					oled_write_string(buff, MENU_FONT, White);
				}

				if((menu->items[index]->flags & MENU_PARAMETER_MASK) == MENU_PARAMETER_IS_STRING){
					oled_set_cursor(COL(posx) + MENU_LEFT_OFFSET + MENU_SELECTOR_SPACING, ROW(i) + MENU_TOP_OFFSET + MENU_TEXT_SPACING*i);
					char buff[64];
					sprintf(buff, "%d", menu->items[index]->parameter);
					oled_write_string(buff, MENU_FONT, White);
				}

			i++;
		}
		  menuLastMenuItem = menuItem;
		  oled_update();
	}else if(menuMode == MENU_MENUMODE_SPLASH){
		(*menu_actual_splash)(menu_actual_splash_param);
		oled_update();
	}

}

void menu_show_splash(void (*callback)(void), void * param){
	menuMode = MENU_MENUMODE_SPLASH;
	menu_actual_splash = callback;
	menu_actual_splash_param = param;
}

void menu_hide_splash(){
	menuMode = MENU_MENUMODE_NORMAL;
	menuForceUpdate = 1;
}

//Funkce vykreslujici zapinaci obrazovku
void menu_start_splash(){
	//Vypisou se texty
	oled_set_cursor((128-(strlen(DEVICE_NAME))*11)/2,10);
	oled_write_string(DEVICE_NAME, Font_11x18, White);

	oled_set_cursor((128-(strlen(DEVICE_TYPE))*11)/2,30);
	oled_write_string(DEVICE_TYPE, Font_11x18, White);

	char * version = "Verze " FW_VERSION;
	oled_set_cursor((128-(strlen(version))*7)/2,50);
	oled_write_string(version, Font_7x10, White);

}

//Funkce pro vypsani chyby
void menu_error_splash(char * msg){
	oled_set_cursor((128-(strlen("Chyba!"))*11)/2, 1);
	oled_write_string("Chyba!", Font_11x18, White);

	oled_set_cursor((128-(strlen(msg))*7)/2, 20);
	oled_write_string(msg, Font_7x10, White);

	if(menuActualSplashKeypress == MENU_KEY_ENTER){
		menuActualSplashKeypress = 0;
		menu_hide_splash();
	}
}
