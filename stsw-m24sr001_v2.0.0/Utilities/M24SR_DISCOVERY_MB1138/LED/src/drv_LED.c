/**
  ******************************************************************************
  * @file    drv_LED.c
  * @author  MMY Application Team
  * @version V1.1
  * @date    15/03/2011
  * @brief   This file provides a set of firmware functions to manage Leds   
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
#if defined STM32F40_41xxx	 
	#include "stm32f4xx.h"
#else
	#include "stm32f10x.h"
#endif
#include "drv_LED.h"

/** @addtogroup Drivers
 * 	@{
 */

/** @addtogroup LED
 * 	@{
 */

/** @defgroup LED_Private_Variables
 *  @{
 */
 
extern GPIO_TypeDef* GPIO_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT, LED4_GPIO_PORT};
extern const uint32_t GPIO_CLK[LEDn] = {LED1_GPIO_CLK, LED2_GPIO_CLK, LED3_GPIO_CLK, LED4_GPIO_CLK};		  
extern const uint16_t GPIO_PIN[LEDn] = {LED1_PIN, LED2_PIN ,LED3_PIN ,LED4_PIN};

uint32_t ledState[LEDn];

/** @defgroup LED_Functions
 *  @{
 */

/**
  * @brief  Configures LED GPIO.
  * @retval None
  */
void LED_Config(Led_TypeDef Led)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	
	/* Enable the GPIO_LED Clock */
#if defined STM32F40_41xxx
	RCC_AHB1PeriphClockCmd(GPIO_CLK[Led], ENABLE);
#else
	RCC_APB2PeriphClockCmd(GPIO_CLK[Led], ENABLE);
#endif
  
  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#if defined STM32F40_41xxx
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
  GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
	
	/* led is not lighting */
	ledState[Led] = 0;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4  
  * @retval None
  */
void LED_On(Led_TypeDef Led)
{
  if(ledState[Led] == 0)
	{	
	#if defined STM32F40_41xxx
		GPIO_PORT[Led]->BSRRL = GPIO_PIN[Led];
	#else
		GPIO_PORT[Led]->BSRR = GPIO_PIN[Led];
	#endif     
		ledState[Led] = 1;	
	}
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4 
  * @retval None
  */
void LED_Off(Led_TypeDef Led)
{
	if(ledState[Led] == 1)
	{
	#if defined STM32F40_41xxx
		GPIO_PORT[Led]->BSRRH = GPIO_PIN[Led];  
	#else
		GPIO_PORT[Led]->BRR = GPIO_PIN[Led];
	#endif
		ledState[Led] = 0;
	}
}

/**
  * @brief  Check if led is On
  * @param  Led: Specifies the Led to be tested. 
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  *     @arg LED4 
  * @retval None
  */
bool IsLED_On(Led_TypeDef Led)
{
	if(ledState[Led] == 1)
		return true;
	else
		return false;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
