/**
  ******************************************************************************
  * @file    drv_bluetooth.c
  * @author  MMY Application Team
  * @version V1.1
  * @date    15/11/2013
  * @brief   This file provides a set of firmware functions to manage BT module   
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
	
/* Includes ------------------------------------------------------------------*/
#include "drv_bluetooth.h"

/** @addtogroup Drivers
 * 	@{
 */

/** @addtogroup BT_Module
 * 	@{
 */

bool BT_ModulePresent = false;

static void BT_CLK_Configuration(void);
static void BT_GPIO_Configuration(void);
static void BT_Reset( void);
static u16 BT_IsDevicePresent(void);
static void BT_SendCommand(char *command);
static uint8_t BT_AnswerReady(void);
static void	BT_GetAnswer( char * pRespbuffer, uint32_t Offset);
static BT_EVENT_MESSAGE BT_IdentifyEvent(char*Event);


volatile char BT_AnswerBuffer[] = "This is the buffer to store the answer of the bluetooth module\0";

uint8_t		BT_Event = 0;
uint32_t  BT_EventSize = 0;
uint8_t		BT_ResponseReady = 0;
uint32_t  BT_ResponseSize = 0;

extern sMediaInfo MediaFile;

/** @defgroup BT_Private_Functions
 *  @{
 */


static void BT_CLK_Configuration(void)
{
  /* Enable GPIO clock */
#if defined STM32F40_41xxx
	RCC_AHB1PeriphClockCmd(BT_GPIO_BLOCK_CLK, ENABLE);
#else
	RCC_APB2PeriphClockCmd(BT_GPIO_BLOCK_CLK, ENABLE);
#endif
  /* Enable UART4 Clock */
  RCC_APB1PeriphClockCmd(BT_UART_BLOCK_CLK, ENABLE);	 
}
  
 
static void BT_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

#if defined STM32F40_41xxx
	/* Configure UART Tx and Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = BT_TX_PIN;
  GPIO_Init(BT_TX_PIN_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = BT_RX_PIN;
  GPIO_Init(BT_RX_PIN_PORT, &GPIO_InitStructure);
#else
	/* Configure UART4 Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = BT_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(BT_TX_PIN_PORT,&GPIO_InitStructure);
   
  /* Configure UART4 Tx as alternate function push-pull   */
  GPIO_InitStructure.GPIO_Pin = BT_RX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(BT_RX_PIN_PORT,&GPIO_InitStructure);
#endif
	
	/* Configure GPIO for BT Reset signal */
  GPIO_InitStructure.GPIO_Pin = BT_RESET_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#if defined STM32F40_41xxx
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
  GPIO_Init(BT_RESET_PIN_PORT, &GPIO_InitStructure);
	
}

static void BT_Reset( void )
{
#if defined STM32F40_41xxx
	/* Pin high state */
	BT_RESET_PIN_PORT->BSRRL = BT_RESET_PIN;  
	delay_ms(1);
	/* Pin low state */
	BT_RESET_PIN_PORT->BSRRH = BT_RESET_PIN;
#else
	/* Pin high state */
	BT_RESET_PIN_PORT->BSRR = BT_RESET_PIN;  
	delay_ms(1);
	/* Pin low state */
	BT_RESET_PIN_PORT->BRR = BT_RESET_PIN;
#endif	
}

static u16 BT_IsDevicePresent(void)
{
	u16 status;
	char Respbuffer[0x40];
	uint8_t timeout=0xFF;
	
	BT_SendCommand("AT*P0\r");
	
	do
	{
		delay_ms(4);
		timeout--;
	}
	while (BT_ResponseReady != 1 && timeout );
	
	if( timeout== 0)
	{
		status = ERROR;
	}
	else
	{	
		BT_ResponseReady = 0;
		status = SUCCESS;
		BT_GetAnswer( (char*)(Respbuffer), 0);
	}
		
	return status;
}

static void BT_SendCommand(char *command)
{
  while(*command)
  {
    while(USART_GetFlagStatus(BT_UART, USART_FLAG_TXE) == RESET);
 
    USART_SendData(BT_UART, *command++);
  }
}

static uint8_t BT_AnswerReady(void)
{
	uint8_t timeout=0xFF;
	
	do
	{
		delay_ms(50);
		timeout--;
	}
	while (BT_ResponseReady != 1 && timeout );
	
	if( timeout== 0)
	{
		return BT_UART_TRANS_FAILED;
	}
	else
	{	
		BT_ResponseReady = 0;
		return BT_UART_TRANS_OK;
	}
}
 
static void	BT_GetAnswer( char * pRespbuffer, uint32_t Offset)
{
	memcpy(pRespbuffer, (const char*)(&BT_AnswerBuffer[Offset]), (BT_ResponseSize-Offset-2));
}

static BT_EVENT_MESSAGE BT_IdentifyEvent(char*Event)
{
	if(!memcmp(Event, PAIRING_SUCCESS, PAIRING_SUCCESS_EVENT) )
		return BT_PAIRING_SUCCESS_EVENT;
	
	else if(!memcmp(Event, PAIRING_FAILED, PAIRING_FAILED_EVENT) )
		return BT_PAIRING_FAILED_EVENT;
	
	else if(!memcmp(Event, HFP_LINK_CONNECTED, HFP_LINK_CONNECTED_EVENT) )
		return BT_HFP_LINK_CONNECTED_EVENT;
	
	else if(!memcmp(Event, A2DP_LINK_CONNECTED, A2DP_LINK_CONNECTED_EVENT) )
		return BT_A2DP_LINK_CONNECTED_EVENT;
	
	else if(!memcmp(Event, SPP_LINK_CONNECTED, SPP_LINK_CONNECTED_EVENT) )
		return BT_SPP_LINK_CONNECTED_EVENT;
	
	else if(!memcmp(Event, PIN_CODE_REQUIERED, PIN_CODE_REQUIERED_EVENT) )
		return BT_PIN_CODE_REQUIERED_EVENT;
	
	else if(!memcmp(Event, CONNECT_TO_LAST_DEVICE, CONNECT_TO_LAST_DEVICE_EVENT) )
		return BT_CONNECT_TO_LAST_DEVICE_EVENT;
		
	else if(!memcmp(Event, HFP_LINK_DISCONNECTED, HFP_LINK_DISCONNECTED_EVENT) )
		return BT_HFP_LINK_DISCONNECTED_EVENT;

	else if(!memcmp(Event, A2DP_LINK_DISCONNECTED, A2DP_LINK_DISCONNECTED_EVENT) )
		return BT_A2DP_LINK_DISCONNECTED_EVENT;
	
	else if(!memcmp(Event, SPP_LINK_DISCONNECTED, SPP_LINK_DISCONNECTED_EVENT) )
		return BT_SPP_LINK_DISCONNECTED_EVENT;
	
	else if(!memcmp(Event, STREAMING_STOPPED, STREAMING_STOPPED_EVENT) )
	{
		/* if song is stopped remove information */
		memset (&MediaFile, 0, sizeof(MediaFile));
		return BT_STREAMING_STOPPED_EVENT;
	}
	
	else if(!memcmp(Event, STREAMING_RESUME, STREAMING_RESUME_EVENT) )
		return BT_STREAMING_RESUME_EVENT;
	
	else if(!memcmp(Event, TITLE_INFO, TITLE_INFO_EVENT) )
	{
		Event +=TITLE_INFO_EVENT;
		memcpy( MediaFile.Title, Event, BT_EventSize-(TITLE_INFO_EVENT));
		return BT_TITLE_INFO_EVENT;
	}
	
	else if(!memcmp(Event, ARTIST_INFO, ARTIST_INFO_EVENT) )
	{
		Event +=ARTIST_INFO_EVENT;
		memcpy( MediaFile.Artist, Event, BT_EventSize-(ARTIST_INFO_EVENT));
		return BT_ARTIST_INFO_EVENT;
	}
	
	else if(!memcmp(Event, ALBUM_INFO, ALBUM_INFO_EVENT) )
	{
		Event +=ALBUM_INFO_EVENT;
		memcpy( MediaFile.Album, Event, BT_EventSize-(ALBUM_INFO_EVENT));
		return BT_ALBUM_INFO_EVENT;
	}
		
	else if(!memcmp(Event, NUMBER_INFO, NUMBER_INFO_EVENT) )
	{
		Event +=NUMBER_INFO_EVENT;
		memcpy( MediaFile.Number, Event, BT_EventSize-(NUMBER_INFO_EVENT));
		return BT_NUMBER_INFO_EVENT;
	}
		
	else if(!memcmp(Event, TOTAL_NUMBER_INFO, TOTAL_NUMBER_INFO_EVENT) )
	{
		Event +=TOTAL_NUMBER_INFO_EVENT;
		memcpy( MediaFile.Total_Number, Event, BT_EventSize-(TOTAL_NUMBER_INFO_EVENT));
		return BT_TOTAL_NUMBER_INFO_EVENT;
	}
	
	else if(!memcmp(Event, GENRE_INFO, GENRE_INFO_EVENT) )
	{
		Event +=GENRE_INFO_EVENT;
		memcpy( MediaFile.Genre, Event, BT_EventSize-(GENRE_INFO_EVENT));
		return BT_GENRE_INFO_EVENT;
	}
	
	else if(!memcmp(Event, PLAYING_TIME_INFO, PLAYING_TIME_INFO_EVENT) )
	{
		Event +=PLAYING_TIME_INFO_EVENT;
		memcpy( MediaFile.PlayingTime, Event, BT_EventSize-(PLAYING_TIME_INFO_EVENT));
		return BT_PLAYING_TIME_INFO_EVENT;
	}
	
	else if(!memcmp(Event, SBC_CODEC, SBC_CODEC_EVENT) )
		return BT_SBC_CODEC_EVENT;
	
	else if(!memcmp(Event, AAC_CODEC, AAC_CODEC_EVENT) )
		return BT_AAC_CODEC_EVENT;
	
	else if(!memcmp(Event, APTX_CODEC, APTX_CODEC_EVENT) )
		return BT_APTX_CODEC_EVENT;
		
	else if(!memcmp(Event, CALL_CONNECTED, CALL_CONNECTED_EVENT) )
		return BT_CALL_CONNECTED_EVENT;
	
	else if(!memcmp(Event, CALL_IDLE, CALL_IDLE_EVENT) )
		return BT_CALL_IDLE_EVENT;
	
	else if(!memcmp(Event, OUTGOING_CALL, OUTGOING_CALL_EVENT) )
		return BT_OUTGOING_CALL_EVENT;
	
	else if(!memcmp(Event, CALL_AUDIO_STOPPED, CALL_AUDIO_STOPPED_EVENT) )
		return BT_CALL_AUDIO_STOPPED_EVENT;
	
	else if(!memcmp(Event, CALL_AUDIO_RESUMED, CALL_AUDIO_RESUMED_EVENT) )
		return BT_CALL_AUDIO_RESUMED_EVENT;
	
	else if(!memcmp(Event, VOLUME_UPDATED, VOLUME_UPDATED_EVENT) )
		return BT_VOLUME_UPDATED_EVENT;
	
	else if(!memcmp(Event, INCOMING_CALL, INCOMING_CALL_EVENT) )
		return BT_INCOMING_CALL_EVENT;
	
	else
		return BT_NO_EVENT;
	
}

/**
  * @}
  */ 

/** @defgroup BT_Public_Functions
 *  @{
 */

u16 BT_Init(void)
{
  USART_InitTypeDef USART_InitStructure;
	char Respbuffer[0x40];

	/* System Clock Configuration */
	BT_CLK_Configuration();
	
#if defined STM32F40_41xxx
	/* Connect UART pins to AF8 */
  GPIO_PinAFConfig(BT_TX_PIN_PORT, GPIO_PinSource11, GPIO_AF_UART4);
  GPIO_PinAFConfig(BT_RX_PIN_PORT, GPIO_PinSource10, GPIO_AF_UART4);
#endif

  /* Configure the GPIO ports */
  BT_GPIO_Configuration();
	
	/* Enable the UART OverSampling by 8 */
  //USART_OverSampling8Cmd(BT_UART, ENABLE);  
	
	/* Reset BT module */
	BT_Reset();
   
  /* UARTx configuration ------------------------------------------------------*/
  /* UARTx configured as follow:
        - BaudRate = 19200 baud 
        - Word Length = 8 Bits
        - Two Stop Bit
        - Odd parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 19200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
 
  USART_Init( BT_UART,&USART_InitStructure);
	
	/* Enable UART4 interrupt */
  USART_ITConfig(BT_UART, USART_IT_RXNE, ENABLE);  
	
	/* Enable the UART4 */
  USART_Cmd(BT_UART, ENABLE);

	
	delay_ms(1500);
	
	/* BT component available only on premium edition */
	/* check if present by disabling pairing at the same time */
	if( BT_IsDevicePresent() == SUCCESS)
	{
		BT_ModulePresent = true;
		BT_SendCommand("AT*AM24SR-DISCOVERY\r");
		BT_AnswerReady();
		BT_GetAnswer( (char*)(Respbuffer), 0);
		
		/* initialize MediaFile structure */
		memset (&MediaFile, 0, sizeof(MediaFile));
	}
	else
	{
		BT_ModulePresent = false;
		return ERROR;
	}
	
	return SUCCESS;

}

bool IsBTModulePresent(void)
{
	return BT_ModulePresent;
}

void BT_Enable(void)
{
	char Respbuffer[10];
	
	/* disable connection on going if any */
	BT_Disable();
	
	BT_SendCommand("AT*P\r");
	BT_AnswerReady();
	BT_GetAnswer( (char*)(Respbuffer), 0);
}

void BT_Disable(void)
{
	char Respbuffer[10];
	
	/* if we disable the current pairing there is no more song information to display */
  memset (&MediaFile, 0, sizeof(MediaFile));
	
	BT_SendCommand("AT*P0\r");
	BT_AnswerReady();
	BT_GetAnswer( (char*)(Respbuffer), 0);
}

void BT_GetMacAddr( char* pMacAddr)
{
	BT_SendCommand("AT*B?\r");
	BT_AnswerReady();
	BT_GetAnswer( pMacAddr, 6);
}

void UART4_IRQHandler(void)
{
  static int rx_index = 0;
	char EventMessage[80];
	
	if (USART_GetITStatus(BT_UART, USART_IT_RXNE) != RESET) // Received characters modify string
  {
    BT_AnswerBuffer[rx_index] = USART_ReceiveData(BT_UART);
		
		if( BT_AnswerBuffer[rx_index] == 0x0A)
		{
			if( BT_AnswerBuffer[rx_index-1] == 0x0D)
			{
				/* if this is an event */
				if(!memcmp((char*)(BT_AnswerBuffer),"IND",3))
				{	
					/* 0x0A & 0x0D not needed but keep 0x0D to replace it by 0x00*/
					BT_EventSize = rx_index;
					
					memcpy((char*)EventMessage,(char*)(BT_AnswerBuffer),BT_EventSize);
					EventMessage[BT_EventSize-1] = 0x00;
					BT_IdentifyEvent(&EventMessage[EVENT_NOTIFICATION_SIZE]);	

					rx_index = 0;						
				}
				else
				{	
					BT_ResponseReady = 1;
					BT_ResponseSize = (rx_index+1);
					rx_index = 0;
				}
			}
		}
		else if (rx_index >= (sizeof(BT_AnswerBuffer) - 1))
      rx_index = 0;
		else
			rx_index++;
  }	
}


/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/

