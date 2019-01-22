/**
	****************** (C) COPYRIGHT 2009 STMicroelectronics **********************
	* @file            main.h
	* @author          MMY Application Team
	* @version         V1.0.0
	* @date            12/20/2013
	* @brief           Header for main.c module
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
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "drv_lcdspi_ili9341.h"
#include "drv_led.h"
#include "drv_bluetooth.h"

#include "lib_TagType4.h"
#include "lib_ISO7816_Password.h"
#include "lib_STProprietary_feature.h"

#include "menu.h"

/* -------------------------------------------------------------------------- 
* Delay TIMER configuration (ms)
* --------------------------------------------------------------------------
* 72 MHz / 72 = 1MHz (1µs)
* 1µs * 1000 + 1µs ~= 1ms	
* -------------------------------------------------------------------------- */ 
#define TIMER_DELAY												TIM2
#define TIMER_DELAY_PERIOD								1000
#define TIMER_DELAY_PRESCALER							72
#define TIMER_DELAY_CLOCK									RCC_APB1Periph_TIM2


/** 
 * @brief  NVIC definitions 
 */
#define TIMER_DELAY_PREEMPTION_PRIORITY						2
#define TIMER_DELAY_SUB_PRIORITY									2
#define TIMER_DELAY_IRQ_CHANNEL										TIM2_IRQn



/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Demo_Init(void);
void InterruptConfig(void);
void SysTick_Configuration(void);
void IntExtOnOffConfig(FunctionalState NewState);
void GPIO_Config(void);
void LedShow_Init(void);
void LedShow(FunctionalState NewState);
uint32_t Get_LedShowStatus(void);
void CheckBitmapFilesStatus(void);

void Lock_M24SR(void);

#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
