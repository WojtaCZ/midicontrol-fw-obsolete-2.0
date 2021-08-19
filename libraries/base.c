#include "base.h"
#include "comm.h"
#include "menu.h"

extern MenuItem menuitem_organ_pwr;
extern uint8_t menuForceUpdate;

void base_init(){
    //Inicializuje proudovy zdroj
    gpio_mode_setup(PORT_CURRENT_SOURCE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_CURRENT_SOURCE);
    gpio_set_output_options(PORT_CURRENT_SOURCE, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_CURRENT_SOURCE);

}

void base_menu_set_current_source(void * m){
    Menu * menu = ((Menu*)m);

    if(!(menu->items[menu->selectedIndex]->flags & MENU_ITEM_IS_CHECKED)){
        base_set_current_source(1);
    }else{
        base_set_current_source(0);
    }

}

void base_set_current_source(uint8_t state){
    if(!!(gpio_port_read(PORT_CURRENT_SOURCE) & GPIO_CURRENT_SOURCE) != state){
        if(state){
            gpio_set(PORT_CURRENT_SOURCE, GPIO_CURRENT_SOURCE);
            menuitem_organ_pwr.flags |= MENU_ITEM_IS_CHECKED;
        }else{
            gpio_clear(PORT_CURRENT_SOURCE, GPIO_CURRENT_SOURCE);
            menuitem_organ_pwr.flags &= ~MENU_ITEM_IS_CHECKED;
        }
    }

    menuForceUpdate = 1;
}

int base_get_current_source(){
	return !!(gpio_port_read(PORT_CURRENT_SOURCE) & GPIO_CURRENT_SOURCE);
}

void base_req_songlist(){
	comm_cmd_send(CMD_GET | MUSIC_SONGLIST , "");
}


//Rutina pro spusteni nahravani
void base_record(uint8_t initiator, char * songname){
	char buffer[] = "set record";
    usb_cdc_tx(buffer, strlen(buffer));
/*
	//Spusteno z PC
	if(initiator == ADDRESS_PC){
		//Jen se zobrazi obrazovka nahravani
		base_set_current_source(1);
		//oled_setDisplayedSplash(oled_recordingSplash, songname);
		
	}else if(initiator == ADDRESS_CONTROLLER){
	//Spusteno ovladacem
		//Jen se zobrazi obrazovka nahravani
		//oled_setDisplayedSplash(oled_recordingSplash, songname);
		//oled_refreshPause();
	}else /*if(initiator == ADDRESS_MAIN)*//*{
	//Spusteno ze zakladnove stanice
		//Posle se zprava do PC aby zacalo nahravat
	/*	char msg[100];
		msg[0] = INTERNAL_COM;
		msg[1] = INTERNAL_COM_REC;
		memcpy(&msg[2], songname, strlen(songname));
		comm_send_msg(ADDRESS_MAIN, ADDRESS_PC, 0, INTERNAL, msg, strlen(songname)+2);
	}
*/

}

void base_play(Menu * menu){
	comm_cmd_send(CMD_SET | MUSIC_PLAY, menu->items[menu->selectedIndex]->text[0]);
	/*char buffer[] = "set play";
    usb_cdc_tx(buffer, strlen(buffer));*/
	//Spusteno z PC
	/*if(initiator == ADDRESS_PC){
		base_set_current_source(1);
		//memset(selectedSong, 0, 40);
		//sprintf(selectedSong, "%s", songname);
		//Jen se zobrazi obrazovka prehravani
		//oled_setDisplayedSplash(oled_playingSplash, songname);
		//oled_refreshPause();
	}else if(initiator == ADDRESS_CONTROLLER){
	//Spusteno ovladacem
		//Jen se zobrazi obrazovka prehravani
		//oled_setDisplayedSplash(oled_playingSplash, songname);
		//oled_refreshPause();
	}else /*if(initiator == ADDRESS_MAIN)*//*{
	//Spusteno ze zakladnove stanice
		//Posle se zprava do PC aby zacalo prehravat
	/*	char msg[100];
		msg[0] = INTERNAL_COM;
		msg[1] = INTERNAL_COM_PLAY;
		memcpy(&msg[2], songname, strlen(songname));
		comm_send_msg(ADDRESS_MAIN, ADDRESS_PC, 0, INTERNAL, msg, strlen(songname)+2);
	}
*/

}

void base_stop(uint8_t initiator){
	//Spusteno z hlavni jednotky
	if(initiator == ADDRESS_MAIN){
		//Posle se zprava do PC o zastaveni
		char msg[2] = {INTERNAL_COM, INTERNAL_COM_STOP};
		comm_send_msg(ADDRESS_MAIN, ADDRESS_PC, 0, INTERNAL, msg, 2);
	}else{
		//Vrati se do menu, zapne OLED refresh a vypne LED
		//oledType = OLED_MENU;
		//oled_refreshResume();
		//setStatusAll(1, DEV_CLR);
	}

}