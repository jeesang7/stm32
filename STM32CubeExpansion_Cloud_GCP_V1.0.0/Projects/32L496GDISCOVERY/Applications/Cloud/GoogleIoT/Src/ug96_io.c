/**
  ******************************************************************************
  * @file    ug96_io.c
  * @author  MCD Application Team
  * @brief   This file implements the IO operations to deal with the C2C
  *          module. It mainly Inits and Deinits the UART interface. Send and
  *          receive data over it.
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

/* Includes ------------------------------------------------------------------*/
#include "ug96_io.h"
#include "ug96_conf.h"
#include "string.h"

/* Private define ------------------------------------------------------------*/
/* intermediate buffer: filled by IRQ and retrieved by the UART_C2C_ReceiveSingleData() */
/* by watching real time, the difference between write_ptr and read ptr is often less then 50 */
#define RING_BUFFER_SIZE                                                   1500


/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  data[RING_BUFFER_SIZE];
  uint16_t tail;
  uint16_t head;
}RingBuffer_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RingBuffer_t UART_RxData;
UART_HandleTypeDef hUART_c2c;

/* Private function prototypes -----------------------------------------------*/
void C2C_ResetSIMConnection(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param hUART_c2c: UART handle pointer
  * @retval None
  */
static void UART_C2C_MspInit(UART_HandleTypeDef *hUART_c2c)
{
  static DMA_HandleTypeDef hdma_tx;
  GPIO_InitTypeDef  GPIO_Init;

  /* Enable the GPIO clock */
  /* C2C_RST_GPIO_CLK_ENABLE(); */


  /* Set the GPIO pin configuration parameters */
  GPIO_Init.Pin       = C2C_RST_PIN;
  GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_HIGH;

  /* Configure the RST IO */
  HAL_GPIO_Init(C2C_RST_GPIO_PORT, &GPIO_Init);
  
  /* Enable DMA clock */
  DMAx_CLK_ENABLE();
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  HAL_PWREx_EnableVddIO2(); /* needed for GPIO PGxx on L496AG*/
  UART_C2C_TX_GPIO_CLK_ENABLE();
  UART_C2C_RX_GPIO_CLK_ENABLE();

  /* Enable UART_C2C clock */
  UART_C2C_CLK_ENABLE();
 
  /* Enable DMA clock */
  DMAx_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_Init.Pin       = UART_C2C_TX_PIN;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Speed     = GPIO_SPEED_HIGH;
  GPIO_Init.Alternate = UART_C2C_TX_AF;

  HAL_GPIO_Init(UART_C2C_TX_GPIO_PORT, &GPIO_Init);

  /* UART RX GPIO pin configuration  */
  GPIO_Init.Pin = UART_C2C_RX_PIN;
  GPIO_Init.Alternate = UART_C2C_RX_AF;
  
  HAL_GPIO_Init(UART_C2C_RX_GPIO_PORT, &GPIO_Init);

  
  /*##-3- Configure the NVIC for UART ########################################*/
  HAL_NVIC_SetPriority(UART_C2C_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(UART_C2C_IRQn);
  
  /*##-4- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = UART_C2C_TX_DMA_CHANNEL;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.Request             = UART_C2C_TX_DMA_REQUEST;

  HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the UART handle */
  __HAL_LINKDMA(hUART_c2c, hdmatx, hdma_tx);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (UART_C2C_TX) */
  HAL_NVIC_SetPriority(UART_C2C_DMA_TX_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(UART_C2C_DMA_TX_IRQn);
  
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hUART_c2c: UART handle pointer
  * @retval None
  */
static void UART_C2C_MspDeInit(UART_HandleTypeDef *hUART_c2c)
{
  static DMA_HandleTypeDef hdma_tx;
  
  /*##-1- Reset peripherals ##################################################*/
  UART_C2C_FORCE_RESET();
  UART_C2C_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(UART_C2C_TX_GPIO_PORT, UART_C2C_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(UART_C2C_RX_GPIO_PORT, UART_C2C_RX_PIN);
  
  /*##-3- Disable the DMA Channels ###########################################*/
  /* De-Initialize the DMA Channel associated to transmission process */
  HAL_DMA_DeInit(&hdma_tx);

  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(UART_C2C_DMA_TX_IRQn);
}


/**
  * @brief Set the pins related to the SIM to input no pull and enable the clocks
  * @retval None
  */
void C2C_ResetSIMConnection()
{
  GPIO_InitTypeDef  GPIO_InitStructSimInputs;

  C2C_SIM_RST_GPIO_CLK_ENABLE();
  C2C_SIM_CLK_GPIO_CLK_ENABLE();
  C2C_SIM_DATA_GPIO_CLK_ENABLE();
  
  /* SIM RESET setting (PC7)*/
  GPIO_InitStructSimInputs.Pin       = C2C_SIM_RST_PIN;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(C2C_SIM_RST_GPIO_PORT, &GPIO_InitStructSimInputs);
  
  /* SIM CLK setting (PA4)*/
  GPIO_InitStructSimInputs.Pin       = C2C_SIM_CLK_PIN;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(C2C_SIM_CLK_GPIO_PORT, &GPIO_InitStructSimInputs);
  
  /* SIM DATA setting (PB12) */
  GPIO_InitStructSimInputs.Pin       = C2C_SIM_DATA_PIN;
  GPIO_InitStructSimInputs.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructSimInputs.Pull      = GPIO_NOPULL;
  GPIO_InitStructSimInputs.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(C2C_SIM_DATA_GPIO_PORT, &GPIO_InitStructSimInputs);
}


/* Public functions ---------------------------------------------------------*/


/**
  * @brief Select which SIM to use
  * @param sim: SIM_EXT_SLOT or SIM_EMBEDDED
  * @retval None
  */
void C2C_SimSelect(C2C_SimSlot_t sim)
{
  GPIO_InitTypeDef  GPIO_InitStructSimSelect;

  C2C_SIM_SEL0_GPIO_CLK_ENABLE();
  C2C_SIM_SEL1_GPIO_CLK_ENABLE();

  C2C_ResetSIMConnection();
  HAL_Delay(50);
  
  /* SIM selection */
  switch( sim )
  {
  case SIM_EXT_SLOT :
    
    /* Sim_select0 */
    /* S */
    /* LOW */
    /* STMOD+, pin 18 (PC2) */
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL0_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL0_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL0_GPIO_PORT, C2C_SIM_SEL0_PIN, GPIO_PIN_RESET);

    /* Sim_select1 */
    /* OE */
    /* LOW */
    /* STMOD+, pin 8 (PI3) */
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL1_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL1_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL1_GPIO_PORT, C2C_SIM_SEL1_PIN, GPIO_PIN_RESET);
    break;
    
   case SIM_EMBEDDED :
  
    /* Sim_select0 */
    /* S */
    /* HIGH */
    /* STMOD+, pin 18 (PC2)*/
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL0_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL0_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL0_GPIO_PORT, C2C_SIM_SEL0_PIN, GPIO_PIN_SET);

    /* Sim_select1 */
    /* OE */
    /* LOW */
    /* STMOD+, pin 8 (PI3) */
    GPIO_InitStructSimSelect.Pin       = C2C_SIM_SEL1_PIN;
    GPIO_InitStructSimSelect.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructSimSelect.Pull      = GPIO_PULLUP;
    GPIO_InitStructSimSelect.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(C2C_SIM_SEL1_GPIO_PORT, &GPIO_InitStructSimSelect);
    HAL_GPIO_WritePin(C2C_SIM_SEL1_GPIO_PORT, C2C_SIM_SEL1_PIN, GPIO_PIN_RESET);
    break;
  }
  
}


/**
  * @brief Hw reset sequence and power up of the modem (it takes about 3 seconds)
  * @retval None
  */
void C2C_HwResetAndPowerUp(void)
{
 GPIO_InitTypeDef  GPIO_InitStructReset, GPIO_InitStructPwr;

  C2C_RST_GPIO_CLK_ENABLE();
  C2C_PWRKEY_GPIO_CLK_ENABLE();
  
  /* RESET sequence */
  /* STMOD+ IO12 */
  /* LOW */
  GPIO_InitStructReset.Pin       = C2C_RST_PIN;
  GPIO_InitStructReset.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructReset.Pull      = GPIO_PULLUP;
  GPIO_InitStructReset.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(C2C_RST_GPIO_PORT, &GPIO_InitStructReset);

  
  /* PWRKEY */
  /* STMOD+ IO9 */
  /* at least LOW during 100 ms */
  GPIO_InitStructPwr.Pin       = C2C_PWRKEY_PIN;
  GPIO_InitStructPwr.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPwr.Pull      = GPIO_PULLUP;
  GPIO_InitStructPwr.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(C2C_PWRKEY_GPIO_PORT,&GPIO_InitStructPwr);

#ifndef USE_BG96
  /* default case is UG96 modem */
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(150);

  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_SET);
  HAL_Delay(150);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_RESET);
  /* Waits for Modem complete its booting procedure */
  HAL_Delay(2300);
#else /* USE_BG96 */
  /* BG96 case */
  /* POWER DOWN */
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(150);
  /* POWER UP */
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(C2C_RST_GPIO_PORT, C2C_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN,GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(C2C_PWRKEY_GPIO_PORT, C2C_PWRKEY_PIN,GPIO_PIN_RESET);
  /* Waits for Modem complete its booting procedure */
  HAL_Delay(5000);
#endif /* USE_BG96 */
}


/**
  * @brief  C2C IO Initalization.
  *         This function inits the UART interface to deal with the C2C,
  *         then starts asynchronous listening on the RX port.
  * @param None
  * @retval 0 on success, -1 otherwise.
  */
int8_t UART_C2C_Init(void)
{
  /* Set the C2C USART configuration parameters on MCU side */
  /* Attention: make sure the module uart is configured with the same values */
  hUART_c2c.Instance        = UART_C2C;
  hUART_c2c.Init.BaudRate   = UG96_DEFAULT_BAUDRATE;
  hUART_c2c.Init.WordLength = UART_WORDLENGTH_8B;
  hUART_c2c.Init.StopBits   = UART_STOPBITS_1;
  hUART_c2c.Init.Parity     = UART_PARITY_NONE;
  hUART_c2c.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  hUART_c2c.Init.Mode       = UART_MODE_TX_RX;
  hUART_c2c.Init.OverSampling = UART_OVERSAMPLING_16;

  UART_C2C_MspInit(&hUART_c2c);
  /* Configure the USART IP */
  if(HAL_UART_Init(&hUART_c2c) != HAL_OK)
  {
    return -1;
  }

  /* Once the C2C UART is initialized, start an asynchronous recursive
   listening. the HAL_UART_Receive_IT() call below will wait until one char is
   received to trigger the HAL_UART_RxCpltCallback(). The latter will recursively
   call the former to read another char.  */
  UART_RxData.head = 0;
  UART_RxData.tail = 0;
  HAL_UART_Receive_IT(&hUART_c2c, (uint8_t *)&UART_RxData.data[UART_RxData.tail], 1);
  
  return 0;
}


/**
  * @brief  C2C IO Deinitialization.
  *         This function Deinits the UART interface of the C2C. When called
  *         the C2C commands can't be executed anymore.
  * @param  None.
  * @retval 0 on success, -1 otherwise.
  */
int8_t UART_C2C_DeInit(void)
{
  /* Reset USART configuration to default */
  HAL_UART_DeInit(&hUART_c2c);
  UART_C2C_MspDeInit(&hUART_c2c);
  
  return 0;
}


/**
  * @brief  C2C IO change baud rate
  *         To be used just in case the UG96_DEFAULT_BAUDRATE need to be changed
  *         This function has to be called after having changed the C2C module baud rate
  *         In order to do that the SMT32 Init shall be done at UG96_DEFAULT_BAUDRATE
  *         After C2C module baud rate is changed this function sets the STM32 baud rate accordingly
  * @param  None.
  * @retval 0 on success, -1 otherwise.
  */
int8_t UART_C2C_SetBaudrate(uint32_t BaudRate)
{
  HAL_UART_DeInit(&hUART_c2c);
  hUART_c2c.Init.BaudRate   = BaudRate;
  if(HAL_UART_Init(&hUART_c2c) != HAL_OK)
  {
    return -1;
  }
  /* Once the C2C UART is initialized, start an asynchronous recursive
   listening. the HAL_UART_Receive_IT() call below will wait until one char is
   received to trigger the HAL_UART_RxCpltCallback(). The latter will recursively
   call the former to read another char.  */
  UART_RxData.head = 0;
  UART_RxData.tail = 0;
  HAL_UART_Receive_IT(&hUART_c2c, (uint8_t *)&UART_RxData.data[UART_RxData.tail], 1);
  
  return 0;
}


/**
  * @brief  Flush Ring Buffer
  * @param  None
  * @retval None.
  */
void UART_C2C_FlushBuffer(void)
{
  memset(UART_RxData.data, 0, RING_BUFFER_SIZE);
  UART_RxData.head = UART_RxData.tail = 0;
}

/**
  * @brief  Send Data to the C2C module over the UART interface.
  *         This function allows sending data to the  C2C Module, the
  *         data can be either an AT command or raw data to send over
  *         a pre-established C2C connection.
  * @param pData: data to send.
  * @param Length: the data length.
  * @retval 0 on success, -1 otherwise.
  */
int16_t UART_C2C_SendData(uint8_t* pData, uint16_t Length)
{
  if (HAL_UART_Transmit_DMA(&hUART_c2c, (uint8_t*)pData, Length) != HAL_OK)
  {
     return -1;
  }

  return 0;
}


/**
  * @brief  Retrieve on Data from intermediate IT buffer
  * @param pData: data to send.
  * @retval 0 data available, -1 no data to retrieve
  */
int16_t  UART_C2C_ReceiveSingleData(uint8_t* pSingleData)
{
  /* Note: other possible implementation is to retrieve directly one data from UART buffer */
  /* without using the interrupt and the intermediate buffer */

  if(UART_RxData.head != UART_RxData.tail)
  {
    /* serial data available, so return data to user */
    *pSingleData = UART_RxData.data[UART_RxData.head++];
    
    /* check for ring buffer wrap */
    if (UART_RxData.head >= RING_BUFFER_SIZE)
    {
      /* ring buffer wrap, so reset head pointer to start of buffer */
      UART_RxData.head = 0;
    }
  }
  else
  {
   return -1;
  }

  return 0;
}


/**
  * @brief  Rx Callback when new data is received on the UART.
  * @param  UartHandle: Uart handle receiving the data.
  * @retval None.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartH)
{
  /* If ring buffer end is reached reset tail pointer to start of buffer */
  if(++UART_RxData.tail >= RING_BUFFER_SIZE)
  {
    UART_RxData.tail = 0;
  }
  
  HAL_UART_Receive_IT(UartH, (uint8_t *)&UART_RxData.data[UART_RxData.tail], 1);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
