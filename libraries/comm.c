#include "comm.h"
#include "usb.h"
#include "scheduler.h"
#include "base.h"

#include <string.h>
#include <stdlib.h>


extern Scheduler sched_comm_decode;

uint8_t commUsbFifo[COMM_USB_FIFO_SIZE];
uint8_t commUsbGotFlag;
uint16_t commUsbMsgLen, commUsbFifoIndex;

uint8_t commUsartFifo[COMM_USART_FIFO_SIZE];

void comm_usb_packet_received(uint8_t * buff, int len){



    if(!commUsbGotFlag) commUsbMsgLen = ((buff[4]&0xff)<<8 | (buff[5]&0xff));

    /*char buffer2[100];
    sprintf(buffer2, "Len: %d Len: %d Len: %d\n", commUsbMsgLen, commUsbMsgLen+6, len);
    usb_cdc_tx(buffer2, strlen(buffer2));
*/
    //Pokud je zprava delsi nez USB packet
    if((commUsbMsgLen+6) > len){
        if(!(buff[0] | buff[1] | buff[2] | buff[3]) && len == 64){
            commUsbFifoIndex = 0;
            memcpy(&commUsbFifo[commUsbFifoIndex], buff, len);
            commUsbFifoIndex += len;
            commUsbGotFlag = 1;
        }else if(commUsbGotFlag){
            //Pokud jeste zbyvaji data na prijeti
            if((commUsbFifoIndex - 6 + len) < commUsbMsgLen  && len == 64){
                //Prekopiruji se data
                memcpy(&commUsbFifo[commUsbFifoIndex], buff, len);
                commUsbFifoIndex += len;
                
            }else if(len >= (commUsbMsgLen+6)-commUsbFifoIndex){
                //Pokud prislo vic dat nez se ocekavalo NEBO je dalsi zprava konecna
                //Prekopiruji se jen ocekavana data
                memcpy(&commUsbFifo[commUsbFifoIndex], buff, (commUsbMsgLen+6)-commUsbFifoIndex);
                //Vyhodnot

                sched_comm_decode.flags = SCHEDULER_ON | SCHEDULER_READY;

            }else{
                commUsbFifoIndex = 0;
                commUsbGotFlag = 0;
            }
        }
    }else if((commUsbMsgLen+6) == len){
        commUsbFifoIndex = 0;
        memcpy(&commUsbFifo[commUsbFifoIndex], buff, commUsbMsgLen+6);
        commUsbFifoIndex += commUsbMsgLen;
        //Vyhodnot
       
        sched_comm_decode.flags = SCHEDULER_ON | SCHEDULER_READY;
        
    }else{
        commUsbFifoIndex = 0;
        commUsbGotFlag = 0;
    }


}

//Rutina pro dekodovani zprav komunikacniho protokolu zarizeni
void comm_decode_callback(){
	//Internal
	uint8_t msgType = commUsbFifo[6];
    uint8_t broadcast = (commUsbFifo[6] & 0x04) >> 2;
	uint8_t src = ((commUsbFifo[6] & 0x18) >> 3);
	uint8_t type = ((msgType & 0xE0) >> 5);

	uint16_t len = (commUsbFifo[4]<<8 | commUsbFifo[5]);

    char buffer[128];
    sprintf(buffer, "Got message! type: %x source: %x broadcast: %x len: %d \n", msgType, src, broadcast, len);
    usb_cdc_tx(buffer, strlen(buffer));

	//if(src == ADDRESS_CONTROLLER) btData = 1;

	if(type == INTERNAL){
		if(commUsbFifo[7] == INTERNAL_COM){
			if(commUsbFifo[8] == INTERNAL_COM_PLAY){
				char * buffPlay = (char*) malloc(50);
				memset(buffPlay, 0, 50);
				memcpy(buffPlay, commUsbFifo+9, len-3);
				base_play(src, buffPlay);
			}else if(commUsbFifo[8] == INTERNAL_COM_STOP){
				base_stop(src);
			}else if(commUsbFifo[8] == INTERNAL_COM_REC){
				char * buffRec = (char*) malloc(50);
				memset(buffRec, 0, 50);
				memcpy(buffRec, commUsbFifo+9, len-3);
				base_record(src, buffRec);
			}else if(commUsbFifo[8] == INTERNAL_COM_KEEPALIVE){
				if(src == ADDRESS_CONTROLLER){
					/*aliveRemote = 1;
					btStreamOpen = 1;
					btStreamSecured = 1;
					btStreamBonded = 1;
					aliveRemoteCounter = 0;*/
				}else if(src == ADDRESS_PC){
					/*alivePC = 1;
					alivePCCounter = 0;*/
				}
			}else if(commUsbFifo[8] == INTERNAL_COM_SET_TIME){
				/*RTC_TimeTypeDef time;
				RTC_DateTypeDef date;

				time.Seconds = commUsbFifo[9];
				time.Minutes = commUsbFifo[10];
				time.Hours = commUsbFifo[11];
				time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
				time.SecondFraction = 0;
				time.SubSeconds = 0;
				time.TimeFormat = RTC_HOURFORMAT_24;
				time.StoreOperation = RTC_STOREOPERATION_RESET;

				date.WeekDay = RTC_WEEKDAY_MONDAY;
				date.Date = commUsbFifo[12];
				date.Month = commUsbFifo[13];
				date.Year = commUsbFifo[14];

				HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
				HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);*/

			}else comm_send_err(0, msgType, len);
		}else if(commUsbFifo[7] == INTERNAL_CURR){
			if(commUsbFifo[8] == INTERNAL_CURR_SET_STATUS){
				if(commUsbFifo[9]){
					base_set_current_source(1);
				}else{
					base_set_current_source(0);
				}

				comm_send_aok(0, msgType, len, 0, NULL);
			}else if(commUsbFifo[8] == INTERNAL_CURR_GET_STATUS){
				//uint8_t currentStatus = HAL_GPIO_ReadPin(CURRENT_SOURCE_GPIO_Port, CURRENT_SOURCE_Pin);
				//comm_send_aok(0, msgType, len, 1, (char*)&currentStatus);
			}else comm_send_err(0, msgType, len);

		}else if(commUsbFifo[7] == INTERNAL_MIDI){
			if(commUsbFifo[8] == INTERNAL_MIDI_GET_STATUS){
				//comm_send_aok(0, msgType, len, 1, (char*)&midiStatus);
			}else if(commUsbFifo[8] == INTERNAL_CURR_GET_STATUS){
				/*uint8_t currentStatus = HAL_GPIO_ReadPin(CURRENT_SOURCE_GPIO_Port, CURRENT_SOURCE_Pin);
				comm_send_aok(0, msgType, len, 1, (char*)&currentStatus);*/
			}else comm_send_err(0, msgType, len);

		}else if(commUsbFifo[7] == INTERNAL_BT){
			if(commUsbFifo[8] == INTERNAL_BT_GET_STATUS){
				/*uint8_t stat = (btStreamOpen << 1) | bluetoothStatus;
				comm_send_aok(0, msgType, len, 1, (char*)&stat);*/
			}else comm_send_err(0, msgType, len);

		}else if(commUsbFifo[7] == INTERNAL_DISP){
			if(commUsbFifo[8] == INTERNAL_DISP_SET_SONG){
				/*numDispSong.enteredValue[0] = commUsbFifo[9];
				numDispSong.enteredValue[1] = commUsbFifo[10];
				numDispSong.enteredValue[2] = commUsbFifo[11];
				numDispSong.enteredValue[3] = commUsbFifo[12];
				workerAssert(&workerDispRefresh);*/
				comm_send_aok(0, msgType, len, 0, NULL);
			}else if(commUsbFifo[8] == INTERNAL_DISP_SET_VERSE){
				/*numDispVerse.enteredValue[0] = commUsbFifo[9];
				numDispVerse.enteredValue[1] = commUsbFifo[10];
				workerAssert(&workerDispRefresh);*/
				comm_send_aok(0, msgType, len, 0, NULL);
			}else if(commUsbFifo[8] == INTERNAL_DISP_SET_LETTER){
				/*numDispLetter.enteredValue[0] = commUsbFifo[9];
				workerAssert(&workerDispRefresh);*/
				comm_send_aok(0, msgType, len, 0, NULL);
			}else if(commUsbFifo[8] == INTERNAL_DISP_SET_LED){
				/*dispLED = commUsbFifo[9];
				workerAssert(&workerDispRefresh);*/
				comm_send_aok(0, msgType, len, 0, NULL);
			}else if(commUsbFifo[8] == INTERNAL_DISP_GET_STATUS){
				//char commUsbFifo[] = {dispConnected, dispSong[3], dispSong[2], dispSong[1], dispSong[0], dispVerse[1], dispVerse[0], dispLetter, (dispLED > 3) ? 0xe0 : dispLED};
				//comm_send_aok(0, msgType, len, 9, commUsbFifo);
			}else comm_send_err(0, msgType, len);
		}else if(commUsbFifo[7] == INTERNAL_USB){
			if(commUsbFifo[8] == INTERNAL_USB_GET_CONNECTED){
				//comm_send_aok(0, msgType, len, 1, (char*)&usbStatus);
			}else comm_send_err(0, msgType, len);

		}else comm_send_err(0, msgType, len);
	}else if(type == EXTERNAL_DISP){
		//midiControl_setDisplayRaw((uint8_t*)&commUsbFifo[7], len-1);
	}else if(type == EXTERNAL_BT){
		//HAL_UART_Transmit_IT(&huart2, (uint8_t*)&commUsbFifo[7], len-1);
	}else if(type == EXTERNAL_MIDI){
		//HAL_UART_Transmit_IT(&huart1, (uint8_t*)&commUsbFifo[7], len-1);
	}else if(type == EXTERNAL_USB){
		//CDC_Transmit_FS((uint8_t*)&commUsbFifo[7], len-1);
	}else if(type == AOKERR){
		//if((commUsbFifo[7] & 0x80) == AOK){

			//Pokud se jedna o odpoved na zpravu z PC do hl. jednotky
			/*if(commUsbFifo[8] == 0x30){
				if(workerGetSongs.assert && workerGetSongs.status == WORKER_WAITING){
					workerGetSongs.status = WORKER_OK;
					strToSongMenu(&commUsbFifo[11], &songMenuSize);
				}

				if(workerRecord.assert && workerRecord.status == WORKER_WAITING){
					if(commUsbFifo[11] == 1){
						workerRecord.status = WORKER_ERR;
					}else{
						workerRecord.status = WORKER_OK;
					}
				}

				}
		}else if((commUsbFifo[7] & 0x80) == ERR){
			if(workerGetSongs.assert && workerGetSongs.status == WORKER_WAITING){
				workerGetSongs.status = WORKER_ERR;
			}
		}*/
	}else{
		comm_send_err(0, msgType, len);
	}


    commUsbFifoIndex = 0;
    commUsbGotFlag = 0;
    sched_comm_decode.flags = 0;
}


//Odesle zpravu typu AOK
void comm_send_aok(uint8_t aokType, uint8_t recType, uint16_t recSize, uint16_t dataSize, char * msg){
	char * buffer = (char*)malloc(dataSize);
	//Utvori se AOK znak s typem
	buffer[0] = 0x80 | (aokType & 0x7f);
	buffer[1] = recType;
	buffer[2] = ((recSize-6) & 0xff00) >> 8;
	buffer[3] = (recSize-6) & 0xff;
	memcpy(&buffer[4], msg, dataSize);
	comm_send_msg(ADDRESS_MAIN, ((recType & 0x18) >> 3), 0, 0x07, buffer, dataSize+4);
	free(buffer);
}

//Odesle zpravu typu ERR
void comm_send_err(uint8_t errType, uint8_t recType, uint16_t recSize){
	char * buffer = (char*)malloc(5);
	//Utvori se ERR znak s typem
	buffer[0] = 0x7f & (errType & 0x7f);
	buffer[1] = recType;
	buffer[2] = ((recSize-6) & 0xff00) >> 8;
	buffer[3] = (recSize-6) & 0xff;

	comm_send_msg(ADDRESS_MAIN, ((recType & 0x18) >> 3), 0, 0x07, buffer, 4);
	free(buffer);
}

//Odesle libovolnou zpravu
void comm_send_msg(uint8_t src, uint8_t dest, uint8_t broadcast, uint8_t type, char * msg, uint16_t len){
	uint8_t * buffer = (uint8_t*)malloc(len+7);
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = ((len+1) >> 4) & 0xff;
	buffer[5] = (len+1) & 0xff;
	buffer[6] = ((type & 0x07) << 5) | ((src & 0x3) << 3) | ((broadcast & 0x01) << 2) | (dest & 0x03);
	memcpy(&buffer[7], msg, len);

	//Podle cile ji odesle na ruzne rozhrani
	if(broadcast){
        usb_cdc_tx(buffer, len+7);
		/*if(btStreamOpen && !btCmdMode){
			HAL_UART_Transmit_IT(&huart2, buffer, len+7);
			btData = 1;
		}*/
	}else if(dest == ADDRESS_PC){
		usb_cdc_tx(buffer, len+7);
	}/*else if(dest == ADDRESS_CONTROLLER && btStreamOpen && !btCmdMode){
		HAL_UART_Transmit_IT(&huart2, buffer, len+7);
		btData = 1;
	}*/

    free(buffer);

}