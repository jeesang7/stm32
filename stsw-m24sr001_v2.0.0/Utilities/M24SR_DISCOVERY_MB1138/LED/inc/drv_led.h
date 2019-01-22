/**
  ******************************************************************************
  * @file    drv_LED.h
  * @author  MMY Application Team
  * @version V1.1
  * @date    15/03/2011
  * @brief   This file provides
  *            - set of firmware functions to manage Leds
  *          available on STM3210B-EVAL evaluation board from STMicroelectronics.    
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRIVER_LED_H
#define __DRIVER_LED_H

#include "stdbool.h"

#ifdef __cplusplus
 extern "C" {
#endif


#define LEDn                             4

#define LED1_PIN                         GPIO_Pin_3
#define LED1_GPIO_PORT                   GPIOC

#define LED2_PIN                         GPIO_Pin_2 
#define LED2_GPIO_PORT                   GPIOC

#define LED3_PIN                         GPIO_Pin_1
#define LED3_GPIO_PORT                   GPIOC
	 
#define LED4_PIN                         GPIO_Pin_0
#define LED4_GPIO_PORT                   GPIOC
	 
#if defined STM32F40_41xxx
	#define LED1_GPIO_CLK                    RCC_AHB1Periph_GPIOC
	#define LED2_GPIO_CLK                    RCC_AHB1Periph_GPIOC
	#define LED3_GPIO_CLK                    RCC_AHB1Periph_GPIOC
	#define LED4_GPIO_CLK                    RCC_AHB1Periph_GPIOC
#else
	#define LED1_GPIO_CLK                    RCC_APB2Periph_GPIOC  
	#define LED2_GPIO_CLK                    RCC_APB2Periph_GPIOC  
	#define LED3_GPIO_CLK                    RCC_APB2Periph_GPIOC  
	#define LED4_GPIO_CLK                    RCC_APB2Periph_GPIOC  
#endif



typedef enum 
{
  LED1 = 0,
	LED2 = 1,
	LED3 = 2,
	LED4 = 3,
	LED5 = 4
} Led_TypeDef;


void LED_Config(Led_TypeDef Led);
void LED_On			(Led_TypeDef Led);
void LED_Off		(Led_TypeDef Led);
bool IsLED_On(Led_TypeDef Led);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_LIB_H */


/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */   
  
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
