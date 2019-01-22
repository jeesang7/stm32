/**
  ******************************************************************************
  * @file    drv_bluetooth.h
  * @author  MMY Application Team
  * @version V1.1
  * @date    15/11/2013
  * @brief   This file contains all the functions prototypes for the BT module 
  *          driver.  
  ******************************************************************************
	* @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
	
#ifndef __DRV_BLUETOOTH_H
#define __DRV_BLUETOOTH_H
	
#include "hw_config.h"

/* -------------------------------- 	UART config	--------------------------------------*/
#define BT_UART                  				UART4
#define BT_UART_BLOCK_CLK								RCC_APB1Periph_UART4

#define BT_RESET_PIN 										GPIO_Pin_3
#define BT_RESET_PIN_PORT 							GPIOA

#define BT_POWERON_PIN 									GPIO_Pin_10
#define BT_POWERON_PIN_PORT 						GPIOA

#define BT_RX_PIN 											GPIO_Pin_10
#define BT_RX_PIN_PORT 									GPIOC

#define BT_TX_PIN 											GPIO_Pin_11
#define BT_TX_PIN_PORT 									GPIOC

#if defined STM32F40_41xxx
#define BT_GPIO_BLOCK_CLK								(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC)
#else
#define BT_GPIO_BLOCK_CLK								(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC)
#endif


/* -------------------------------- 	Protocol	----------------------------------------*/
#define BT_UART_TRANS_OK								1
#define BT_UART_TRANS_FAILED						0

/* ---------------------------------- 	Event	 -----------------------------------------*/
#define EVENT_NOTIFICATION							"IND"
#define EVENT_NOTIFICATION_SIZE					3

#define PAIRING_SUCCESS									"*P1"
#define PAIRING_FAILED									"*P0"
#define HFP_LINK_CONNECTED							":*C0"
#define A2DP_LINK_CONNECTED							":*C1"
#define SPP_LINK_CONNECTED							":*C2"
#define PIN_CODE_REQUIERED							"*N"
#define CONNECT_TO_LAST_DEVICE					":*B"
#define HFP_LINK_DISCONNECTED						":*Z0"
#define A2DP_LINK_DISCONNECTED					":*Z1"
#define SPP_LINK_DISCONNECTED						":*Z2"
#define STREAMING_STOPPED								":-M1"
#define STREAMING_RESUME								":-M0"
#define TITLE_INFO											":-A1"
#define ARTIST_INFO											":-A2"
#define ALBUM_INFO											":-A3"
#define NUMBER_INFO											":-A4"
#define TOTAL_NUMBER_INFO								":-A5"
#define GENRE_INFO											":-A6"
#define PLAYING_TIME_INFO								":-A7"
#define SBC_CODEC												":-C1"
#define AAC_CODEC												":-C2"
#define APTX_CODEC											":-C3"
#define CALL_CONNECTED									":+C"
#define CALL_IDLE												":+I"
#define OUTGOING_CALL										":+O"
#define CALL_AUDIO_STOPPED							":+B0"
#define CALL_AUDIO_RESUMED							":+B1"
#define VOLUME_UPDATED									":+V"
#define INCOMING_CALL										":+S"

#define PAIRING_SUCCESS_EVENT						3
#define PAIRING_FAILED_EVENT						3
#define HFP_LINK_CONNECTED_EVENT				4
#define A2DP_LINK_CONNECTED_EVENT				4
#define SPP_LINK_CONNECTED_EVENT				4
#define PIN_CODE_REQUIERED_EVENT				2
#define CONNECT_TO_LAST_DEVICE_EVENT		3
#define HFP_LINK_DISCONNECTED_EVENT			4
#define A2DP_LINK_DISCONNECTED_EVENT		4
#define SPP_LINK_DISCONNECTED_EVENT			4
#define STREAMING_STOPPED_EVENT					4
#define STREAMING_RESUME_EVENT					4
#define TITLE_INFO_EVENT								4
#define ARTIST_INFO_EVENT								4
#define ALBUM_INFO_EVENT								4
#define NUMBER_INFO_EVENT								4
#define TOTAL_NUMBER_INFO_EVENT					4
#define GENRE_INFO_EVENT								4
#define PLAYING_TIME_INFO_EVENT					4
#define SBC_CODEC_EVENT									4
#define AAC_CODEC_EVENT									4
#define APTX_CODEC_EVENT								4
#define CALL_CONNECTED_EVENT						3
#define CALL_IDLE_EVENT		      				3
#define OUTGOING_CALL_EVENT							3
#define CALL_AUDIO_STOPPED_EVENT				4
#define CALL_AUDIO_RESUMED_EVENT				4
#define VOLUME_UPDATED_EVENT						3
#define INCOMING_CALL_EVENT							3

typedef enum{
	BT_NO_EVENT = 0,
	BT_PAIRING_SUCCESS_EVENT,
	BT_PAIRING_FAILED_EVENT,
	BT_HFP_LINK_CONNECTED_EVENT,
	BT_A2DP_LINK_CONNECTED_EVENT,
	BT_SPP_LINK_CONNECTED_EVENT,
	BT_PIN_CODE_REQUIERED_EVENT,
	BT_CONNECT_TO_LAST_DEVICE_EVENT,
	BT_HFP_LINK_DISCONNECTED_EVENT,
	BT_A2DP_LINK_DISCONNECTED_EVENT,
	BT_SPP_LINK_DISCONNECTED_EVENT,
	BT_STREAMING_STOPPED_EVENT,
	BT_STREAMING_RESUME_EVENT,
	BT_TITLE_INFO_EVENT,
	BT_ARTIST_INFO_EVENT,
	BT_ALBUM_INFO_EVENT,
	BT_NUMBER_INFO_EVENT,
	BT_TOTAL_NUMBER_INFO_EVENT,
	BT_GENRE_INFO_EVENT,
	BT_PLAYING_TIME_INFO_EVENT,
	BT_SBC_CODEC_EVENT,
	BT_AAC_CODEC_EVENT,
	BT_APTX_CODEC_EVENT,
	BT_CALL_CONNECTED_EVENT,
	BT_CALL_IDLE_EVENT,
	BT_OUTGOING_CALL_EVENT,
	BT_CALL_AUDIO_STOPPED_EVENT,
	BT_CALL_AUDIO_RESUMED_EVENT,
	BT_VOLUME_UPDATED_EVENT,
	BT_INCOMING_CALL_EVENT
}BT_EVENT_MESSAGE; 


typedef struct
{
  char Title[52];
  char Artist[52];
	char Album[52];
	char Number[8];
	char Total_Number[8];
	char Genre[52];
	char PlayingTime[20];
}sMediaInfo;

 
u16 BT_Init(void);
bool IsBTModulePresent(void);
void BT_Enable(void);
void BT_Disable(void);

void BT_GetMacAddr( char* pMacAddr);

//BT_EVENT_MESSAGE BT_CheckEvent(char*pEventBuffer);

#endif

