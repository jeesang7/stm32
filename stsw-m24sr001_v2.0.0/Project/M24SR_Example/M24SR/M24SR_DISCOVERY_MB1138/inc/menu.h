/**
	****************** (C) COPYRIGHT 2009 STMicroelectronics **********************
	* @file            menu.h
	* @author          MMY Application Team
	* @version         V1.0.0
	* @date            12/20/2013
	* @brief           This file contains all the functions prototypes for the
	*                  menu navigation firmware driver.
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
#ifndef __MENU_H
#define __MENU_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/* Private types ------------------------------------------------------------*/
/* Private constants --------------------------------------------------------*/
#define J_SEL_PIN				GPIO_Pin_6
#define J_SEL_PORT			GPIOB
#define J_SEL_CLOCK			RCC_APB2Periph_GPIOB

#define J_DOWN_PIN			GPIO_Pin_7
#define J_DOWN_PORT			GPIOB
#define J_DOWN_CLOCK		RCC_APB2Periph_GPIOB

#define J_LEFT_PIN			GPIO_Pin_5
#define J_LEFT_PORT			GPIOB
#define J_LEFT_CLOCK		RCC_APB2Periph_GPIOB

#define J_RIGHT_PIN			GPIO_Pin_8
#define J_RIGHT_PORT		GPIOB
#define J_RIGHT_CLOCK		RCC_APB2Periph_GPIOB

#define J_UP_PIN				GPIO_Pin_9
#define J_UP_PORT				GPIOB
#define J_UP_CLOCK			RCC_APB2Periph_GPIOB

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define  MAX_MENU_LEVELS 4
#define  NOKEY  0
#define  SEL    1
#define  RIGHT  2
#define  LEFT   3
#define  UP     4
#define  DOWN   5
#define  KEY    6

#define NO_EVENT   0
#define RF_EVENT 	 1
#define KEY_EVENT  2

/* Module private variables --------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define countof(a) (sizeof(a) / sizeof(*(a)))

/* Private functions ---------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void STIntro(void);
void InitJoystick(void);
void ShowMenuIcons(void);
void UpFunc(void);
void DownFunc(void);
void SelFunc(void);

#endif /* __MENU_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
