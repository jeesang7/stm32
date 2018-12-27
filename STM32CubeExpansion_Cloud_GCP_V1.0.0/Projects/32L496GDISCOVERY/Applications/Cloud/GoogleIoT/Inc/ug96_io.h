/**
  ******************************************************************************
  * @file    ug96_io.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the C2C IO operations.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UG96_IO_H
#define UG96_IO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
  SIM_EXT_SLOT,
  SIM_EMBEDDED
} C2C_SimSlot_t;

/* Exported constants --------------------------------------------------------*/
/* This section can be used to tailor UART_C2C instance used and associated
   resources */
#define UART_C2C                           USART1
#define UART_C2C_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE();
#define DMAx_CLK_ENABLE()                  __HAL_RCC_DMA1_CLK_ENABLE()
#define UART_C2C_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define UART_C2C_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define UART_C2C_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define UART_C2C_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for UART_C2C Pins */
#define UART_C2C_TX_PIN                    GPIO_PIN_6
#define UART_C2C_TX_GPIO_PORT              GPIOB
#define UART_C2C_TX_AF                     GPIO_AF7_USART1

/* PortG on L496AG device needs Independent I/O supply rail;
   It can be enabled by setting the IOSV bit in the PWR_CR2 register,
   when the VDDIO2 supply is present (depends by the package).*/
#define UART_C2C_RX_PIN                    GPIO_PIN_10
#define UART_C2C_RX_GPIO_PORT              GPIOG
#define UART_C2C_RX_AF                     GPIO_AF7_USART1

#define UART_C2C_RTS_PIN                   GPIO_PIN_12
#define UART_C2C_RTS_GPIO_PORT             GPIOG
#define UART_C2C_RTS_AF                    GPIO_AF7_USART1

#define UART_C2C_CTS_PIN                   GPIO_PIN_11
#define UART_C2C_CTS_GPIO_PORT             GPIOG
#define UART_C2C_CTS_AF                    GPIO_AF7_USART1

/* Definition for UART_C2C's NVIC IRQ and IRQ Handlers */
#define UART_C2C_IRQn                      USART1_IRQn
//#define UART_C2C_IRQHandler                USART1_IRQHandler

/* Definition for UART_C2C's DMA */
#define UART_C2C_TX_DMA_CHANNEL            DMA1_Channel4
/* Definition for UART_C2C's DMA Request */
#define UART_C2C_TX_DMA_REQUEST            DMA_REQUEST_2
/* Definition for UART_C2C's NVIC */
#define UART_C2C_DMA_TX_IRQn               DMA1_Channel4_IRQn
//#define UART_C2C_DMA_TX_IRQHandler         DMA1_Channel4_IRQHandler

/* C2C module Reset pin definitions */
#define C2C_RST_PIN                        GPIO_PIN_2
#define C2C_RST_GPIO_PORT                  GPIOB
#define C2C_RST_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOB_CLK_ENABLE()

/* C2C module PowerKey pin definitions */
#define C2C_PWRKEY_PIN                     GPIO_PIN_3
#define C2C_PWRKEY_GPIO_PORT               GPIOD
#define C2C_PWRKEY_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOD_CLK_ENABLE()

/* C2C SIM Reset pin definitions */
#define C2C_SIM_RST_PIN                    GPIO_PIN_7
#define C2C_SIM_RST_GPIO_PORT              GPIOC
#define C2C_SIM_RST_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

/* C2C SIM Clock pin definitions */
#define C2C_SIM_CLK_PIN                    GPIO_PIN_4
#define C2C_SIM_CLK_GPIO_PORT              GPIOA
#define C2C_SIM_CLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

/* C2C SIM DATA pin definitions */
#define C2C_SIM_DATA_PIN                   GPIO_PIN_12
#define C2C_SIM_DATA_GPIO_PORT             GPIOB
#define C2C_SIM_DATA_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()

/* C2C SIM Selection_0 pin definitions */
#define C2C_SIM_SEL0_PIN                   GPIO_PIN_2
#define C2C_SIM_SEL0_GPIO_PORT             GPIOC
#define C2C_SIM_SEL0_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()

/* C2C SIM Selection_1 pin definitions */
#define C2C_SIM_SEL1_PIN                   GPIO_PIN_3
#define C2C_SIM_SEL1_GPIO_PORT             GPIOI
#define C2C_SIM_SEL1_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOI_CLK_ENABLE()

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void C2C_SimSelect(C2C_SimSlot_t sim);
void C2C_HwResetAndPowerUp(void);


int8_t  UART_C2C_Init(void);
int8_t  UART_C2C_DeInit(void);
int8_t  UART_C2C_SetBaudrate(uint32_t BaudRate);
void    UART_C2C_FlushBuffer(void);
int16_t UART_C2C_SendData(uint8_t* Buffer, uint16_t Length);
int16_t UART_C2C_ReceiveSingleData(uint8_t* pData);

#ifdef __cplusplus
}
#endif

#endif /* UG96_IO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
