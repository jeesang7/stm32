/**
  ******************************************************************************
  * @file    st_device.h
  * @author  STMicroelectronics
  * @version V1.0
  * @date    11-March-2016
  * @brief   This file contains all the functions prototypes for metrology
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ST_DEVICE_H
#define __ST_DEVICE_H

#ifdef __cplusplus
 extern "C" {
#endif
 
   
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Communication */
#define SPI_STPM                    SPI1
#define SPI_STPM_SPEED              SPI_BAUDRATEPRESCALER_16
#define SPI_TIMEOUT		    10

#define USART_STPM                  USART3
#define USART_SPEED		    57600
#define USART_TIMEOUT		    10
   
/* GPIOs */
#define CS_GPIO_type                GPIOA
#define CS_GPIO_pin                 GPIO_PIN_4
#define SYN_GPIO_type               GPIOA
#define SYN_GPIO_pin                GPIO_PIN_9
#define LED2_GPIO_type              GPIOA
#define LED2_GPIO_pin               GPIO_PIN_0

/**** No Enable pin used in case of IPE023 but if you can use it */
/* used here only to mimic the use case */
#define EN_GPIO_type                GPIOA
#define EN_GPIO_pin                 GPIO_PIN_3
    
   
/*----------------------------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __ST_DEVICE_H */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
