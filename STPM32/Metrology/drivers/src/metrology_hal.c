/**
  ******************************************************************************
  * @file    metrology_hal.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    11-March-2016
  * @brief   This file provides The hardware abstraction for Metrology Block.
  * @brief
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
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "stpm_metrology.h"
#include "metrology.h"
#include "metrology_hal.h"
#include <string.h>
#include <stdint.h>
#include "st_device.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define METRO_BUFF_COM_MAXSIZE  ((uint8_t)(40))

#define CRC_8 (0x07)
#define STPM3x_FRAME_LEN (5)

#define WAIT_DURATION   500   /* 500 * 1 ms = 500 ms */

#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr If expr is false, it calls assert_failed function
  *   which reports the name of the source file and the source
  *   line number of the call that failed.
  *   If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint8_t CRC_u8Checksum;

/* Private function prototypes -----------------------------------------------*/
#ifdef UART_XFER_STPM3X /* UART MODE */  
static void Metro_HAL_usart_config(uint32_t in_baudrate);
static void Metro_HAL_UsartTxStart(METRO_NB_Device_t in_Metro_Device_Id);
static uint8_t UARTWrp_SendAndReceiveByte(UART_HandleTypeDef *huart,uint8_t data);
static uint8_t Metro_HAL_byteReverse(uint8_t in_byte);
#endif

static void Metro_HAL_Crc8Calc (uint8_t in_Data);
static uint8_t Metro_HAL_CalcCRC8(uint8_t *pBuf);
static void Metro_HAL_WaitMicroSecond(uint32_t time);
static void Metro_HAL_reset_transfer_context(METRO_NB_Device_t in_Metro_Device_Id);
static void Metro_HAL_STPM_SYN_reset_3_pulses(METRO_NB_Device_t in_Metro_Device_Id);
static void Metro_HAL_STPM_SYN_single_pulse(METRO_NB_Device_t in_Metro_Device_Id);
static void Metro_HAL_CSS_EXT_Device(METRO_NB_Device_t in_Metro_Device_Id,GPIO_PinState in_Metro_enable);
static void Metro_HAL_EN_EXT_Device(METRO_NB_Device_t in_Metro_Device_Id,GPIO_PinState in_Metro_enable);

static void Metro_HAL_RxHandler(void);
static void Metro_HAL_TxHandler(void);

#ifdef SPI_XFER_STPM3X /* SPI MODE */ 
static void Metro_HAL_Spi_config(void);
static void Metro_HAL_SpiTxStart(METRO_NB_Device_t in_Metro_Device_Id);
static uint8_t SPIWrp_SendAndReceiveByte(SPI_HandleTypeDef *hspi,uint8_t data);
#endif


/* Global variables ----------------------------------------------------------*/
volatile uint32_t waitDummyCounter;

METRO_Device_Config_t * p_Metro_Device_Config;
uint8_t Metro_Com_TxBuf[METRO_BUFF_COM_MAXSIZE];
uint8_t Metro_Com_RxBuf[METRO_BUFF_COM_MAXSIZE];

UART_HandleTypeDef huart;
SPI_HandleTypeDef hspi;


/* Private functions ---------------------------------------------------------*/

/** @defgroup Metrology_HAL_Private_Functions
* @{
*/

/** @defgroup Metrology_Group1 Initialization and Configuration functions
*  @brief   Initialization and Configuration functions
*
@verbatim
===============================================================================
                functions
===============================================================================

This section provides a set of functions to make the porting of Metrology Block from STCOMET or STMET or STPM

@endverbatim
* @{
*/


/**
  * @brief  HAL  metrology Config
  *
  *
  * @retval U32
  */
/* set metrology HAL Config  */
uint8_t Metro_HAL_Setup(METRO_Device_Config_t * in_p_Metro_Config)
{
  /* Save the pointer of Config table */
  p_Metro_Device_Config = in_p_Metro_Config;

  return 0;

}


/**
  * @brief  Read Block Registers from device
  *
  *
  * @retval void
  */
uint8_t Metrology_HAL_ReadBlock(METRO_NB_Device_t in_Metro_Device_Id, uint8_t Offset, uint8_t BlockNum, uint32_t * out_p_Buffer)
{
  uint32_t tmp_addr = 0x0;
  uint8_t  error=0;

  if ( in_Metro_Device_Id < NB_MAX_DEVICE)
   {
      /* Calculate the base address to read inisde STPM chip  */
     /* the offset should be provided (2 bytes -> 16 bits) format for STPM */
      tmp_addr = (uint32_t)&METRO_STPM->DSPCTRL1 + Offset;

      /* read blocks from external chip */
      Metro_HAL_Stpm_Read(in_Metro_Device_Id,(uint8_t*)&tmp_addr,BlockNum,out_p_Buffer);


   }
   return error;
}
/**
  * @brief  Write Block Registers to device
  *
  *
  * @retval void
  */
uint8_t Metrology_HAL_WriteBlock(METRO_NB_Device_t in_Metro_Device_Id, uint8_t Offset, uint8_t BlockNum, uint32_t * in_p_Buffer)
{
  uint32_t tmp_addr = 0x0;
  uint32_t ret_size;

  /* Calculate the base address to read inisde STPM chip  */
  /* the offset should be provided in 2 bytes format (16 bits by 16 bits) for STPM */
  tmp_addr = (uint32_t)&METRO_STPM->DSPCTRL1 + (Offset);

   /* write blocks from external chip */
  ret_size = Metro_HAL_Stpm_write(in_Metro_Device_Id,(uint8_t*)&tmp_addr,BlockNum,in_p_Buffer,STPM_WAIT); 
  
  return(ret_size);
}


/**
  * @brief  Init Metrology devices
  *
  *
  * @retval void
  */
void Metro_HAL_init_device(METRO_NB_Device_t in_Metro_Device_Id)
{
 
#ifdef UART_XFER_STPM3X /* UART MODE */   
    Metro_HAL_usart_config(9600); 
#endif    

#ifdef SPI_XFER_STPM3X /* UART MODE */   
    Metro_HAL_Spi_config();
#endif 
  
    /* set good CS with good EXT chip */
    Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
    Metro_HAL_WaitMicroSecond(1000); 
 
    /* Reset EXT Chip */
    /* 3 pulses on SYN */
    Metro_HAL_STPM_SYN_reset_3_pulses(in_Metro_Device_Id);
           
    /* Single pulse on CS rev BC */        
    Metro_HAL_WaitMicroSecond(1000); 
    Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
    Metro_HAL_WaitMicroSecond(100); 
    Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
      
   /* Wait 20 ms */
   Metro_HAL_WaitMicroSecond(20000);   
   
   /* Make one access ( first reg u32 reg) to ext chip to check if it is available */
   
   if (Metro_HAL_Stpm_Read(in_Metro_Device_Id,0,1,&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL1)==0)
   {
    // Reset Config for EXT chip 
    p_Metro_Device_Config[in_Metro_Device_Id].device   = Device_NONE;
    p_Metro_Device_Config[in_Metro_Device_Id].channels_mask = 0;

   }
  
}

/**
  * @brief  Enable Metrology devices by setting up and down the EN pin
  *         When EN is low the CSS is set low or high depending of the 
  *         protocol used (SPI or UART)
  * @retval void
  */

void Metro_HAL_power_up_device(METRO_NB_Device_t in_Metro_Device_Id)
{
  
#ifdef UART_XFER_STPM3X /* UART MODE */   
  /* set UART mode at STPM3x power up, we have to set SS pin */
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
#endif

#ifdef SPI_XFER_STPM3X /* SPI MODE */   
  /* set SPI mode at STPM3x power up, we have to reset SS pin */
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET); 
#endif

  /* set ENable Pin configured as low in CubeMX*/
  Metro_HAL_EN_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
  Metro_HAL_WaitMicroSecond(1000); 
  Metro_HAL_EN_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
  Metro_HAL_WaitMicroSecond(1000); 
}

/**
  * @brief      This function reset device requested
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @param[in]   Reset type
  * @param[out]  none
  * @retval
  */
void Metro_HAL_reset_device(METRO_ResetType_t in_MetroResetType,METRO_NB_Device_t in_Metro_Device_Id)
{
  uint32_t tmp_addr = 0;

  if(in_Metro_Device_Id < NB_MAX_DEVICE)
  {
    if (in_MetroResetType ==  RESET_SW)
    {
      /* Set the reset bit in the  DSP Control Register 3 of stpm requested(STPM) */
      /* First put the bit inside internal struct */
      p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 |= METRO_STPM_Reset_Bit;

      /* second : write into external the chip */
        /* Calculate the base address to read inisde STPM chip  */
        /* the offset should be provided in 2 bytes format (16 bits by 16 bits) for STPM */
      tmp_addr = (uint32_t)((uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 - (uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL1)/2;

     /* Write blocks inside external chip */
      Metro_HAL_Stpm_write(in_Metro_Device_Id,(uint8_t*)&tmp_addr,1,&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3,STPM_WAIT);


    }
    /* reset SYN hardware is requested, send 3 pulses to SYN signal pin */
    else if (in_MetroResetType ==  RESET_SYN_SCS)
    {
    /* Reset ext STPMs with syn PIN : 3 pulses are needed to reset STPM chips */
    Metro_HAL_STPM_SYN_reset_3_pulses(in_Metro_Device_Id);
    }
  }
}

/**
  * @brief      This function set latch inside Metrology devices
               Latch the device registers according to the latch type selection driving SYN pin
               or writing S/W Latchx bits in the DSP_CR3 register
               or setting auto-latch by S/W Auto Latch bit in the DSP_CR3 register
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @param[in]   Latch type
  * @param[out]  none
  * @retval
  */
void Metro_HAL_Set_Latch_device_type(METRO_NB_Device_t in_Metro_Device_Id, METRO_Latch_Device_Type_t in_Metro_Latch_Device_Type)
{
  uint32_t tmp_addr = 0;

  if(in_Metro_Device_Id < NB_MAX_DEVICE)
  {
    switch (in_Metro_Latch_Device_Type)
    {

      case LATCH_AUTO:
      {

        /* reset latch 1 and 2 bits in the internal DSP Control Register 3 */
        p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 &= ~ BIT_MASK_STPM_LATCH1;
        p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 &= ~ BIT_MASK_STPM_LATCH2;


        /* Set  latch auto in the internal DSP Control Register 3*/
        p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 |= BIT_MASK_STPM_AUTO_LATCH;

        /* Now send data to the external chip */
        /* Calculate the base address to read inisde STPM chip  */
        /* the offset should be provided in 2 bytes format (16 bits by 16 bits) for STPM */
        tmp_addr = (uint32_t)((uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 - (uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL1)/2;

        /* Write register inside external chip */
        Metro_HAL_Stpm_write(in_Metro_Device_Id,(uint8_t*)&tmp_addr,1,&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3,STPM_WAIT);

      }
      break;
      case LATCH_SW:
      {
        /* Set  latch SW 1 et 2 for the Two channels  the internal DSP Control Register 3*/
        p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 |= BIT_MASK_STPM_LATCH1|BIT_MASK_STPM_LATCH2;

        /* Now send data to the external chip */
        /* Calculate the base address to read inisde STPM chip  */
        /* the offset should be provided in 2 bytes format (16 bits by 16 bits) for STPM */
        tmp_addr = (uint32_t)((uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3 - (uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL1)/2;

        /* Write register inside external chip */
        Metro_HAL_Stpm_write(in_Metro_Device_Id,(uint8_t*)&tmp_addr,1,&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL3,STPM_WAIT);

      }
      break;
      case LATCH_SYN_SCS:
      {
        /* Latch external chip with syn PIN : 1 pulses is needed to latch */
        Metro_HAL_STPM_SYN_single_pulse(in_Metro_Device_Id);
      }
      break;
    }
     
    
     /* After latch with syn pin or SW reg , we have to retreive metrology data from STPM external chip requested */
    /* from DSPEVENT1 to TOT_REG4 : 49 U32 reg from STPM*/
    /* Calculate the base address of Metrology data to read inisde STPM chip  */
    /* the offset should be provided in 2 bytes format (16 bits by 16 bits) for STPM */
    tmp_addr = (uint32_t)((uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.UARTSPISR - (uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL1)/2;

    Metro_HAL_Stpm_Read(in_Metro_Device_Id,(uint8_t*)&tmp_addr,METRO_STPM_DATA_REG_NB_BLOCKS,&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.UARTSPISR);

  }

}

/**
  * @brief      This function read the Power of signal come from one Channel mapped in one device
* @param[in]   in_Metro_Device_Id : EXT1 to EXT4
  * @param[in]   in_Metro_int_Channel (Channel ID), Channel 1 or 2 ( according to device )
  * @param[in]   in_Metro_Power_Selection : W_ACTIVE , F_ACTIVE, REACTIVE, APPARENT_RMS, APPARENT_VEC, MOM_WIDE_ACT, MOM_FUND_ACT
  * @param[out]  None
  * @retval     int32_t raw_power according to power type  read inside device register
  */

int32_t Metro_HAL_read_power(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel,METRO_Power_selection_t in_Metro_Power_Selection)
{
  int32_t raw_power = 0;

  switch (in_Metro_Power_Selection)
  {
    case (W_ACTIVE):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG5)&BIT_MASK_STPM_PRIM_CURR_ACTIVE_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG5)&BIT_MASK_STPM_SEC_CURR_ACTIVE_POW);
      }
    }
    break;
    case (F_ACTIVE):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG6)&BIT_MASK_STPM_PRIM_CURR_FUND_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG6)&BIT_MASK_STPM_SEC_CURR_FUND_POW);
      }
    }
    break;
    case (REACTIVE):
    {
      /* is Channel is the first or the second  channel of the chip */
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG7)&BIT_MASK_STPM_PRIM_CURR_REACT_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG7)&BIT_MASK_STPM_SEC_CURR_REACT_POW);
       }
    }
    break;
    case (APPARENT_RMS):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG8)&BIT_MASK_STPM_PRIM_CURR_RMS_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG8)&BIT_MASK_STPM_SEC_CURR_RMS_POW);
      }
    }
    break;
    case (APPARENT_VEC):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG9)&BIT_MASK_STPM_PRIM_CURR_VEC_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG9)&BIT_MASK_STPM_SEC_CURR_VEC_POW);
      }
    }
    break;
    case (MOM_WIDE_ACT):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG10)&BIT_MASK_STPM_PRIM_CURR_MOM_ACTIVE_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG10)&BIT_MASK_STPM_SEC_CURR_MOM_ACTIVE_POW);
      }
    }
    break;
    case (MOM_FUND_ACT):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG11)&BIT_MASK_STPM_PRIM_CURR_MOM_FUND_POW);
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
         /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_power = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG11)&BIT_MASK_STPM_SEC_CURR_MOM_FUND_POW);
      }
    }
    break;
  } /* end switch */

  raw_power <<= 4;  // handle sign extension as power is on 28 bits
  raw_power >>= 4;

  return raw_power;
}

/**
  * @brief      This function read the Energy of signal come from one Channel mapped in one device
  * @param[in]   in_Metro_Device_Id (device ID), HOST or EXT1 to EXT4
  * @param[in]   in_Metro_int_Channel (Channel ID), Channel 1 or 2 ( according to device )
  * @param[in]   in_Metro_Energy_Selection : E_W_ACTIVE , E_F_ACTIVE, E_REACTIVE, E_APPARENT
  * @param[out]  None
  * @retval     int32_t raw_nrj according to Energy type  read inside device register
  */

int64_t Metro_HAL_read_energy(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel,METRO_Energy_selection_t in_Metro_Energy_Selection)
{
  int32_t raw_nrj = 0;

  switch (in_Metro_Energy_Selection)
  {
    case (E_W_ACTIVE):
    {
    if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
       /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG1));
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG1));
      }
    }
    break;
    case (E_F_ACTIVE):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG2));
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG2));
      }
    }
    break;
    case (E_REACTIVE):
    {
      if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = -((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG3));
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = -((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG3));
      }
    }
    break;
    case (E_APPARENT):
    {
    if (in_Metro_int_Channel == INT_CHANNEL_1)
      {
        /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH1_REG4));
      }
      else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
      {
        /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
        raw_nrj = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.CH2_REG4));
      }
    }
    break;
  } /* end switch */

  return raw_nrj;
}

/**
  * @brief      This function read the momentary Cuurent from one device and one channel 
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @param[in]   in_Metro_int_Channel (Channel ID), Channel 1, 2 of device
  * @param[out]  None
  * @retval     uint32_t raw RMS voltage
  */
uint32_t Metro_HAL_read_RMS_Voltage(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel)
{
  uint32_t RMS_voltage = 0;

  if (in_Metro_int_Channel == INT_CHANNEL_1)
  {
    /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
    RMS_voltage = (p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSP_REG14)&BIT_MASK_STPM_DATA_VRMS;
  }
  else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
  {
    /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
    RMS_voltage = (p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSP_REG15)&BIT_MASK_STPM_DATA_VRMS;
  }
  
  return (RMS_voltage);
}

/**
  * @brief      This function read the momentary Cuurent from one device and one channel 
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @param[in]   in_Metro_int_Channel (Channel ID), Channel 1, 2 of device
  * @param[out]  None
  * @retval     uint32_t raw RMS current
  */
uint32_t Metro_HAL_read_RMS_Current(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel)
{
  uint32_t RMS_current = 0;

  if (in_Metro_int_Channel == INT_CHANNEL_1)
  {
    /* get directly from RAM, be carrefull : latch should be made before to have coherents values inside DSP data reg */
    RMS_current = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSP_REG14)&BIT_MASK_STPM_DATA_C1_RMS)>>BIT_MASK_STPM_DATA_C_RMS_SHIFT;
  }
  else if (in_Metro_int_Channel == INT_CHANNEL_2)/* is channel 2 */
  {
    /* get directly from RAM, be carrefull !!!!! : latch should be made before to have coherents values inside DSP data reg */
    RMS_current = ((p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSP_REG15)&BIT_MASK_STPM_DATA_C2_RMS)>>BIT_MASK_STPM_DATA_C_RMS_SHIFT;
  }
  
  return (RMS_current);
}

/*************************************************************************/
/*        Porting wrapper UART Interface to dialog with STPM             */
/*************************************************************************/

/*******************/
/* LOCAL FUNCTIONS */
/*******************/


/**
  * @brief  This function send 1 pulse on SYN signal to latch metrology registers of STPM external chips
  * @param  METRO_NB_Device_t in_Metro_Device_Id
  * @retval None
  */
static void Metro_HAL_STPM_SYN_single_pulse(METRO_NB_Device_t in_Metro_Device_Id)
{

#ifdef UART_XFER_STPM3X /* UART MODE */   
  
    /* Before to toogle SYN pin , we have to Clear  SS  pin ( Chip select to Ext device ) */
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);

#endif
  
  HAL_GPIO_WritePin(SYN_GPIO_type,SYN_GPIO_pin,GPIO_PIN_SET);

  /* reset SYNC pulse */
  HAL_GPIO_WritePin(SYN_GPIO_type,SYN_GPIO_pin,GPIO_PIN_RESET);

  Metro_HAL_WaitMicroSecond(100); 
  /* set SYNC pulse */
  HAL_GPIO_WritePin(SYN_GPIO_type,SYN_GPIO_pin,GPIO_PIN_SET);
  Metro_HAL_WaitMicroSecond(100); 
  
#ifdef UART_XFER_STPM3X /* UART MODE */ 
  
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
  
#endif

}

/**
  * @brief  This function send 3 pulses on SYN signal to golbal reset STPM external chips
  * @param  METRO_NB_Device_t in_Metro_Device_Id
  * @retval None
  */
static void Metro_HAL_STPM_SYN_reset_3_pulses(METRO_NB_Device_t in_Metro_Device_Id)
{

#ifdef UART_XFER_STPM3X /* UART MODE */   
  
  /* Before to toogle SYN pin , we have to Clear  SS  pin ( Chip select to Ext device ) */
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);

#endif
  
  HAL_GPIO_WritePin(SYN_GPIO_type,SYN_GPIO_pin,GPIO_PIN_SET);
  
  for(uint8_t i=0;i<=2; i++) 
  {
    /* reset SYNC pulse */
    HAL_GPIO_WritePin(SYN_GPIO_type,SYN_GPIO_pin,GPIO_PIN_RESET);

    Metro_HAL_WaitMicroSecond(100); 
    /* set SYNC pulse */
    HAL_GPIO_WritePin(SYN_GPIO_type,SYN_GPIO_pin,GPIO_PIN_SET);
    Metro_HAL_WaitMicroSecond(100); 

  }

#ifdef UART_XFER_STPM3X /* UART MODE */ 
  
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
  Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
  
#endif
}

/**
  * @brief      This function reset transfer context for external chip Only
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @retval     None
  */
static void Metro_HAL_reset_transfer_context(METRO_NB_Device_t in_Metro_Device_Id)
{
  /* Reset Fields transfer */

   /* TX side */
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxReadBuf  = Metro_Com_TxBuf;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxWriteBuf = Metro_Com_TxBuf;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txValid = 0;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing = 0;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txData = 0;

   /* Rx Side */
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pRxReadBuf = Metro_Com_RxBuf;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pRxWriteBuf = Metro_Com_RxBuf;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxValid = 0;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxOngoing = 0;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxData = 0;

}


/**
  * @brief  This function handles wait
  * @param  time : time to wait in microseconds
  * @retval None
  */
static void Metro_HAL_WaitMicroSecond(uint32_t time)
{
  int i;
  uint32_t tmp;
#define ACCURACY               8
#define ADJ_RATIO              104
#define OVERHEAD               3
  if (SystemCoreClock == 96000000)
  {
    tmp = time * ((4<<ACCURACY)*ADJ_RATIO/100);
    tmp >>= ACCURACY;
  }
  else if (SystemCoreClock == 72000000)
  {
    tmp = time * ((3<<ACCURACY)*ADJ_RATIO/100);
    tmp >>= ACCURACY;
  }
  else if (SystemCoreClock == 48000000)
  {
    tmp = time * ((2<<ACCURACY)*ADJ_RATIO/100);
    tmp >>= ACCURACY;
  }
  else // default is 24000000
  {
    tmp = time * ((1<<ACCURACY)*ADJ_RATIO/100);
    tmp >>= ACCURACY;
  }
  if (tmp >= OVERHEAD)
  {
    tmp -= OVERHEAD;
  }
  for (i=0; i<tmp; i++)
  {
    waitDummyCounter++;
  }
}

#ifdef UART_XFER_STPM3X /* UART MODE */   
/**
  * @brief      Reverse byte
  * @param      one Byte
  * @retval     Byte reversed
  */
static uint8_t Metro_HAL_byteReverse(uint8_t in_byte)
{
    in_byte = ((in_byte >> 1) & 0x55) | ((in_byte << 1) & 0xaa);
    in_byte = ((in_byte >> 2) & 0x33) | ((in_byte << 2) & 0xcc);
    in_byte = ((in_byte >> 4) & 0x0F) | ((in_byte << 4) & 0xF0);

    return in_byte;
}
#endif 
/**
  * @brief      Calculate CRC of a byte
  * @param      one Byte
  * @retval     None
  */
static void Metro_HAL_Crc8Calc (uint8_t in_Data)
{
    uint8_t loc_u8Idx;
    uint8_t loc_u8Temp;
    loc_u8Idx=0;
    while(loc_u8Idx<8)
    {
        loc_u8Temp=in_Data^CRC_u8Checksum;
        CRC_u8Checksum<<=1;
        if(loc_u8Temp&0x80)
        {
            CRC_u8Checksum^=CRC_8;
        }
        in_Data<<=1;
        loc_u8Idx++;
    }
}
/**
  * @brief      Calculate CRC of a frame
  * @param      Buf Frame
  * @retval     u8 checksum of the frame
  */
static uint8_t Metro_HAL_CalcCRC8(uint8_t *pBuf)
{
    uint8_t     i;
    CRC_u8Checksum = 0x00;

    for (i=0; i<STPM3x_FRAME_LEN-1; i++)
    {
        Metro_HAL_Crc8Calc(pBuf[i]);
    }

    return CRC_u8Checksum;
}


/**
  * @brief        This function trigs a read of a set of U32 reg (frame) from ext devices
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @param[in]   Base adress of data to write (U16 base address for STPM),
  * @param[in]   Nb of blocks (nb of u32 to write )
  *@param[in]    in_wait_stpm : 0 : no wait RX frame from STPM after the write 1 : wait RX frame from STPM
  * @param[in]  in_p_Buffer : Data to write
  * @retval
  */
uint32_t Metro_HAL_Stpm_write(METRO_NB_Device_t in_Metro_Device_Id,uint8_t * in_p_data,uint8_t nb_blocks,uint32_t * in_p_Buffer,uint8_t in_wait_stpm)
{
   uint32_t retSize = 0;
   uint8_t nb_blocks_tmp;
#ifdef UART_XFER_STPM3X /* UART MODE */  
   uint8_t CRC_on_reversed_buf;
#endif
   uint8_t i=0;
   uint8_t *p_writePointer = (uint8_t*)in_p_Buffer;
   uint8_t k=0;

   uint8_t frame_with_CRC[STPM3x_FRAME_LEN];
   uint8_t frame_without_CRC[STPM3x_FRAME_LEN -1];


    /* Reset Buffers */
   memset(Metro_Com_TxBuf,0,METRO_BUFF_COM_MAXSIZE);
   memset(Metro_Com_RxBuf,0,METRO_BUFF_COM_MAXSIZE);

   /* Reset Fields transfer */
   Metro_HAL_reset_transfer_context(in_Metro_Device_Id);


   /* Format the Frames according to NB blocks to write */
   /* One block is an U32 but STPM need to write in two times ( 2 U16 )*/
   /* Nb blocks * 2 to multiply by 2 the write requets */
   if (nb_blocks==0)
   {
     nb_blocks_tmp = 1;
   }
   else
   {
     nb_blocks_tmp = nb_blocks*2;
   }
   
   for (k=0;k<nb_blocks_tmp;k++)
   {

     /* Format the frame with Write base address */
     frame_with_CRC[0] = 0xff; /*  No read requested, put dummy frame  */
     frame_with_CRC[1] = (*in_p_data) + k; /*  write Address requested */
     frame_with_CRC[2] = *(p_writePointer); /*   DATA for 16-bit register to be written, LSB */
     frame_with_CRC[3] = *(++p_writePointer); /*  DATA for 16-bit register to be written, MSB */

     /* Increment Pointer to next U16 data for the next loop */
     p_writePointer++;

#ifdef UART_XFER_STPM3X /* UART MODE */ 
     /* Reverse bytes */
     for (i=0;i<(STPM3x_FRAME_LEN-1);i++)
     {
       frame_without_CRC[i] = Metro_HAL_byteReverse(frame_with_CRC[i]);
     }

     /* Calculate CRC and put it at the end of the frame */
     CRC_on_reversed_buf = Metro_HAL_CalcCRC8(frame_without_CRC);
     frame_with_CRC[4] = Metro_HAL_byteReverse(CRC_on_reversed_buf);
#endif

#ifdef SPI_XFER_STPM3X     
     for (i=0;i<(STPM3x_FRAME_LEN-1);i++)
     {
       frame_without_CRC[i] = frame_with_CRC[i];
     }  
     frame_with_CRC[4] = Metro_HAL_CalcCRC8(frame_without_CRC);
     
#endif
     
     
     /* Put the frame inside the TX queue      */
     memcpy(p_Metro_Device_Config[EXT1].STPM_com.pTxWriteBuf,&frame_with_CRC,STPM3x_FRAME_LEN);

     retSize = retSize + STPM3x_FRAME_LEN;
     p_Metro_Device_Config[EXT1].STPM_com.pTxWriteBuf+= STPM3x_FRAME_LEN;

     
#ifdef UART_XFER_STPM3X /* UART MODE */      
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET); 

     /* Send  Data */
     Metro_HAL_UsartTxStart(in_Metro_Device_Id);
#endif
     
#ifdef SPI_XFER_STPM3X     
     /* toggle CSS signal to level 0 of good EXT chip during Full duplex transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET); 
     
     /* Send  Data */
     Metro_HAL_SpiTxStart(in_Metro_Device_Id);
#endif

     /* Wait STPM RX reception frame for STPM after a write */
     if (in_wait_stpm == STPM_WAIT)
     {
        /* Wait end of RX frame reception to go to next tx frame*/
        while (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxValid < 1);        
     }
     
     
#ifdef SPI_XFER_STPM3X /* SPI MODE */ 
      /* toggle CSS signal to level 1 end of transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET); 
#endif 
       
     
   /* Reset Fields transfer */
   Metro_HAL_reset_transfer_context(in_Metro_Device_Id);


   } /* end For Nb blocks loop */


   /* Reset Fields transfer */
   Metro_HAL_reset_transfer_context(in_Metro_Device_Id);


   /* Reset RX/TX Buffers */
   memset(Metro_Com_TxBuf,0,METRO_BUFF_COM_MAXSIZE);
   memset(Metro_Com_RxBuf,0,METRO_BUFF_COM_MAXSIZE);


   return(retSize);
}

/**
  * @brief      This function trigs a read of a set of U32 reg (frame) from ext devices
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @param[in]   Base adress (U16 for STPM),
  * @param[in]  Nb of blocks (nb of u32 to read )
  * @param[out]  output buffer filled
  * @retval
  */
uint32_t Metro_HAL_Stpm_Read(METRO_NB_Device_t in_Metro_Device_Id,uint8_t * in_p_data,uint8_t nb_blocks,uint32_t * out_p_read_data)
{
   uint32_t retSize = 0;
#ifdef UART_XFER_STPM3X /* UART MODE */  
   uint8_t CRC_on_reversed_buf;
#endif   
   uint8_t i =0;
   uint8_t k=0;
   uint8_t * p_read_data = (uint8_t*) out_p_read_data;

   uint8_t frame_with_CRC[STPM3x_FRAME_LEN];
   uint8_t frame_without_CRC[STPM3x_FRAME_LEN -1];

   /* init struct of device requested*/
   //memset(p_Metro_Device_Config[in_Metro_Device_Id],0,sizeof(METRO_Device_Config_t));

   /* Reset Buffers */
   memset(Metro_Com_TxBuf,0,METRO_BUFF_COM_MAXSIZE);
   memset(Metro_Com_RxBuf,0,METRO_BUFF_COM_MAXSIZE);

   /* Reset Fields transfer */
   Metro_HAL_reset_transfer_context(in_Metro_Device_Id);

   /* First frame send the data read pointer inside frame */
   /* Format the frame with Read base address */
   frame_with_CRC[0] = (*in_p_data); /* put the read adress */
   frame_with_CRC[1] = 0xFF; /* no write requested */
   frame_with_CRC[2] = 0xFF; /* no Data */
   frame_with_CRC[3] = 0xFF; /* no Data */
  
#ifdef UART_XFER_STPM3X /* UART MODE */ 
     /* Reverse bytes */
     for (i=0;i<(STPM3x_FRAME_LEN-1);i++)
     {
       frame_without_CRC[i] = Metro_HAL_byteReverse(frame_with_CRC[i]);
     }

     /* Calculate CRC and put it at the end of the frame */
     CRC_on_reversed_buf = Metro_HAL_CalcCRC8(frame_without_CRC);
     frame_with_CRC[4] = Metro_HAL_byteReverse(CRC_on_reversed_buf);
#endif 
     
#ifdef SPI_XFER_STPM3X     
     for (i=0;i<(STPM3x_FRAME_LEN-1);i++)
     {
       frame_without_CRC[i] = frame_with_CRC[i];
     }  
     frame_with_CRC[4] = Metro_HAL_CalcCRC8(frame_without_CRC);
     
#endif   
   
   
   
   /* Put the frame inside the TX queue      */
   memcpy(p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxWriteBuf,&frame_with_CRC,STPM3x_FRAME_LEN);

   retSize = retSize + STPM3x_FRAME_LEN;
   p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxWriteBuf+= STPM3x_FRAME_LEN;

#ifdef UART_XFER_STPM3X /* UART MODE */      
     /* toggle CSS signal to level 1 of good EXT chip during Full duplex transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);     
     /* Send  Data */
     Metro_HAL_UsartTxStart(in_Metro_Device_Id);
#endif
     
#ifdef SPI_XFER_STPM3X     
     /* toggle CSS signal to level 0 of good EXT chip during Full duplex transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);     
     
     /* Send  Data */
     Metro_HAL_SpiTxStart(in_Metro_Device_Id);
#endif


   volatile uint32_t tmp2 = 0;
        
   /* Wait end of RX frame reception to go to next tx frame and check if anwser is not too long : detect no ext chip connected  */
   while (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxValid < 1)
   {

     tmp2++;
       
     if (tmp2 > WAIT_DURATION)
     {
        Metro_HAL_reset_transfer_context(in_Metro_Device_Id);
        
        return 0;
     }    
   }  
#ifdef SPI_XFER_STPM3X /* SPI MODE */    
     /* toggle CSS signal to level 1 after transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);   
#endif   


   /* Reset Fields transfer */
   Metro_HAL_reset_transfer_context(in_Metro_Device_Id);

   /* Format the Frames according to NB blocks to read, one block is one U32 register inside STPM */
   for (k=0;k<nb_blocks;k++)
   {

     /* send FF to read next frame*/

    /* Format the frame with Read base address */
     frame_with_CRC[0] = 0xFF; /* put FF to read */
     frame_with_CRC[1] = 0xFF; /* no write requested */
     frame_with_CRC[2] = 0xFF; /* no Data */
     frame_with_CRC[3] = 0xFF; /* no Data */

#ifdef UART_XFER_STPM3X /* UART MODE */ 
     /* Reverse bytes */
     for (i=0;i<(STPM3x_FRAME_LEN-1);i++)
     {
       frame_without_CRC[i] = Metro_HAL_byteReverse(frame_with_CRC[i]);
     }

     /* Calculate CRC and put it at the end of the frame */
     CRC_on_reversed_buf = Metro_HAL_CalcCRC8(frame_without_CRC);
     frame_with_CRC[4] = Metro_HAL_byteReverse(CRC_on_reversed_buf);
#endif
     
#ifdef SPI_XFER_STPM3X     
     for (i=0;i<(STPM3x_FRAME_LEN-1);i++)
     {
       frame_without_CRC[i] = frame_with_CRC[i];
     }  
     frame_with_CRC[4] = Metro_HAL_CalcCRC8(frame_without_CRC);
     
#endif

     /* Put the frame inside the TX queue      */
     memcpy(p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxWriteBuf,&frame_with_CRC,STPM3x_FRAME_LEN);

     retSize = retSize + STPM3x_FRAME_LEN;
     p_Metro_Device_Config[EXT1].STPM_com.pTxWriteBuf+= STPM3x_FRAME_LEN;

#ifdef UART_XFER_STPM3X /* UART MODE */      
     /* toggle CSS signal to level 1 of good EXT chip during Full duplex transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);     
     /* Send  Data */
     Metro_HAL_UsartTxStart(in_Metro_Device_Id);
#endif
     
#ifdef SPI_XFER_STPM3X     
     /* toggle CSS signal to level01 of good EXT chip during Full duplex transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_RESET);
     
     /* Send  Data */
     Metro_HAL_SpiTxStart(in_Metro_Device_Id);
#endif

     /* Wait end of RX frame reception to go to next tx frame*/
     while (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxValid < 1);

#ifdef SPI_XFER_STPM3X /* SPI MODE */        
     /* toggle CSS signal to 0 of good EXT chip after Full duplex transfert */
     Metro_HAL_CSS_EXT_Device(in_Metro_Device_Id,GPIO_PIN_SET);
#endif

    /* Now Retreive RX data (one frame of 4 bytes : one U32): memcpy from RX buffer */
    /* first increment pointer of data and put new data at the end of buffer */
    memcpy(p_read_data+(k*4), p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pRxReadBuf,4);

    /* Reset Fields transfer */
    Metro_HAL_reset_transfer_context(in_Metro_Device_Id);

   } /* end For Nb blocks loop */

   /* Reset Buffers */
   memset(Metro_Com_TxBuf,0,METRO_BUFF_COM_MAXSIZE);
   memset(Metro_Com_RxBuf,0,METRO_BUFF_COM_MAXSIZE);

   return(retSize);
}

/**
  * @brief  CSS management for External Metrology Devices
  *
  *
  * @retval void
  */
static void Metro_HAL_CSS_EXT_Device(METRO_NB_Device_t in_Metro_Device_Id,GPIO_PinState in_Metro_enable)
{    
  if ( in_Metro_Device_Id == EXT1)
  {
     /* Set chip select of Ext chip */  
	HAL_GPIO_WritePin(CS_GPIO_type,CS_GPIO_pin,in_Metro_enable);     
  }
}

/**
  * @brief  EN management for External Metrology Devices
  *         It is used in case of EN can be controlled
  *
  * @retval void
  */
static void Metro_HAL_EN_EXT_Device(METRO_NB_Device_t in_Metro_Device_Id,GPIO_PinState in_Metro_enable)
{    
  if ( in_Metro_Device_Id == EXT1)
  {
     /* Set chip select of Ext chip */  
	HAL_GPIO_WritePin(EN_GPIO_type,EN_GPIO_pin,in_Metro_enable);     
  }
}


/**
  * @brief      handles  RX data
  * @param      None
  * @retval     None
  */
static void Metro_HAL_RxHandler(void)
{
  static uint8_t Cpt_char;

  if (p_Metro_Device_Config[EXT1].STPM_com.pRxWriteBuf == p_Metro_Device_Config[EXT1].STPM_com.pRxReadBuf)
  {
    p_Metro_Device_Config[EXT1].STPM_com.rxValid = 0;
    p_Metro_Device_Config[EXT1].STPM_com.rxOngoing = 0;
    p_Metro_Device_Config[EXT1].STPM_com.pRxReadBuf = Metro_Com_RxBuf;
    p_Metro_Device_Config[EXT1].STPM_com.pRxWriteBuf = Metro_Com_RxBuf;
    Cpt_char = 0;

    /* get the first u8 of the first frame */
    *p_Metro_Device_Config[EXT1].STPM_com.pRxWriteBuf = p_Metro_Device_Config[EXT1].STPM_com.rxData;
    p_Metro_Device_Config[EXT1].STPM_com.pRxWriteBuf += 1;
    p_Metro_Device_Config[EXT1].STPM_com.rxOngoing = 1;
    Cpt_char++;
  }
  else
  {
    Cpt_char ++;

    /* if frame is not completed  (5 bytes) continue to received bytes */
    if (Cpt_char <= 4)
    {
      *p_Metro_Device_Config[EXT1].STPM_com.pRxWriteBuf = p_Metro_Device_Config[EXT1].STPM_com.rxData;
      p_Metro_Device_Config[EXT1].STPM_com.pRxWriteBuf += 1;
      p_Metro_Device_Config[EXT1].STPM_com.rxOngoing = 1;

    }
    /* if 5 Bytes are received  : 4 data bytes + 1 CRC byte)*/
    else if (Cpt_char > 4)
    {
      /* it is the last Char of frame => CRC, trash it for the moment */
      /* No copy inside the buffer */

      /* increment RX Nb frame Valid */
      p_Metro_Device_Config[EXT1].STPM_com.rxValid++;

      /* reset CPT char, go to Next frame if necessary*/
      Cpt_char = 0;
    }
  }

}

/**
  * @brief      handles TX data
  * @param      None
  * @retval     None
  */
static void Metro_HAL_TxHandler(void)
{
  if (p_Metro_Device_Config[EXT1].STPM_com.pTxWriteBuf == p_Metro_Device_Config[EXT1].STPM_com.pTxReadBuf)
  {
    p_Metro_Device_Config[EXT1].STPM_com.txValid = 0;
    p_Metro_Device_Config[EXT1].STPM_com.txOngoing = 0;
    p_Metro_Device_Config[EXT1].STPM_com.pTxReadBuf = Metro_Com_TxBuf;
    p_Metro_Device_Config[EXT1].STPM_com.pTxWriteBuf = Metro_Com_TxBuf;
  }
  else
  {
    p_Metro_Device_Config[EXT1].STPM_com.txData = *p_Metro_Device_Config[EXT1].STPM_com.pTxReadBuf;
    p_Metro_Device_Config[EXT1].STPM_com.pTxReadBuf += 1;
    p_Metro_Device_Config[EXT1].STPM_com.txValid = 1;
  }
}


/*****************************/
/* USART Interface FUNCTIONS */
/*****************************/

#ifdef UART_XFER_STPM3X /* UART MODE */   

/**
  * @brief  USART Config for External Metrology Devices
  *
  *
  * @retval void
  */

static void Metro_HAL_usart_config(uint32_t in_baudrate)
{
  huart.Instance = USART_STPM;
  huart.Init.BaudRate = in_baudrate;
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits = UART_STOPBITS_1;
  huart.Init.Parity = UART_PARITY_NONE;
  huart.Init.Mode = UART_MODE_TX_RX;
  huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart);
}

/**
  * @brief      handles first TX data
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @retval     None
  */
static void Metro_HAL_UsartTxStart(METRO_NB_Device_t in_Metro_Device_Id)
{  
  uint8_t data;

  huart.Instance = USART_STPM;

  /* Enable the Peripheral clear pending error if any*/
  __HAL_UART_ENABLE(&huart);
  Metro_HAL_WaitMicroSecond(100);

  if (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing == 0)
  {  
    data  = UARTWrp_SendAndReceiveByte(&huart,*p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxReadBuf);
    p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxReadBuf += 1;
    p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing = 1;  
    
    do
    {
      p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxData = data;
      Metro_HAL_RxHandler();

      if (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxWriteBuf != 0)
      {
        Metro_HAL_TxHandler();

        if (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txValid==1)
        {
          data  = UARTWrp_SendAndReceiveByte(&huart,p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txData);
        }
      }
    }while(p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing);

  }
   /* Disable the Peripheral */
  __HAL_UART_DISABLE(&huart);

}


/**
  * @brief      Send and receive byte through UART
  * @param[in]  in_Metro_Device_Id (device ID), EXT1 to EXT4 and Data to send
  * @retval     Data recieved
  */

static uint8_t UARTWrp_SendAndReceiveByte(UART_HandleTypeDef *huart,uint8_t data)
{

  HAL_UART_Transmit(huart, &data, 1, 0);
  HAL_UART_Receive(huart, &data, 1, USART_TIMEOUT);
  return(data);

}


/**
  * @brief      This function set the baud rate of HOST between host and STPM
  * @brief      !!!! Be Carrefull, change baud rate of external chip before to change baud rate of HOST...
  * @param[in]  in_Metro_Device_Id: Device ID
  * @param[in]  in_baudrate: new baudrate to transmit
  * @retval     return true if the baudsate is change, False if not because transmission is on going in STPM side
  */
uint8_t Metro_HAL_baudrate_set(METRO_NB_Device_t in_Metro_Device_Id,uint32_t in_baudrate)
{
    uint32_t tmp_addr = 0;
    static uint32_t tmp_data = 0;
    
    uint32_t stpm_baudrate = 0;
    
    if (in_Metro_Device_Id == HOST)
    {
       Metro_HAL_usart_config(in_baudrate);
       return 1;
    }    
    else if (in_Metro_Device_Id == EXT1)
    {
       switch (in_baudrate)
       {
        case 2400 :
          stpm_baudrate = METRO_STPM_UART_BAUDRATE_2400;
          break;
        case 9600 :
          stpm_baudrate = METRO_STPM_UART_BAUDRATE_9600;
          break;
        case 19200 :
          stpm_baudrate = METRO_STPM_UART_BAUDRATE_19200;
          break;

        case 57600 :
          stpm_baudrate = METRO_STPM_UART_BAUDRATE_57600;
          break;

        case 115200 :
           stpm_baudrate = METRO_STPM_UART_BAUDRATE_115200;
          break;

        case 230400 :
           stpm_baudrate = METRO_STPM_UART_BAUDRATE_230400;
          break;

        case 460800 :
          stpm_baudrate = METRO_STPM_UART_BAUDRATE_460800;
          break;

       default :
         return 0;
         break;

       }

        /* Set  new baudrate according to request*/
        p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.UARTSPICR2 = stpm_baudrate;

        /* Now send data to the external chip */
        /* Calculate the base address to read inside STPM chip  */
        /* the offset should be provided in 2 bytes format (16 bits by 16 bits) for STPM */
        tmp_addr = (uint32_t)((uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.UARTSPICR2 - (uint8_t*)&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.DSPCTRL1)/2;

        Metro_HAL_Stpm_Read(in_Metro_Device_Id,(uint8_t*)&tmp_addr,1,&tmp_data);

        /* Write register inside external chip with "no wait" option because the return frame will not be at the same baudrate */
        Metro_HAL_Stpm_write(in_Metro_Device_Id,(uint8_t*)&tmp_addr,0,&p_Metro_Device_Config[in_Metro_Device_Id].metro_stpm_reg.UARTSPICR2,STPM_NO_WAIT);

        Metro_HAL_WaitMicroSecond(10000);
      
        return 1;
    }
    else
    {
        return 0;
    }

}
#endif


/*****************************/
/* SPI  Interface FUNCTIONS */
/*****************************/

#ifdef SPI_XFER_STPM3X /* SPI MODE */   

/**
  * @brief  SPI Config for External Metrology Devices
  *
  *
  * @retval void
  */

static void Metro_HAL_Spi_config(void)
{
  hspi.Instance = SPI_STPM;
  hspi.Init.Mode = SPI_MODE_MASTER;
  hspi.Init.Direction = SPI_DIRECTION_2LINES;
  hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi.Init.NSS = SPI_NSS_SOFT;
  hspi.Init.BaudRatePrescaler = SPI_STPM_SPEED;
  hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi.Init.CRCPolynomial = 7;
  HAL_SPI_Init(&hspi);
}

/**
  * @brief      handles first TX data
  * @param[in]   in_Metro_Device_Id (device ID), EXT1 to EXT4
  * @retval     None
  */
static void Metro_HAL_SpiTxStart(METRO_NB_Device_t in_Metro_Device_Id)
{  
  uint8_t data;
      
  /* Enable SPI peripheral */   
  __HAL_SPI_ENABLE(&hspi);
  Metro_HAL_WaitMicroSecond(100);
  
  if (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing == 0)
  {  
    data  = SPIWrp_SendAndReceiveByte(&hspi,*p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxReadBuf);
    p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxReadBuf += 1;
    p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing = 1;  
    
    do
    {
      p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.rxData = data;
      Metro_HAL_RxHandler();

      if (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pTxWriteBuf != 0)
      {
        Metro_HAL_TxHandler();

        if (p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txValid==1)
        {
          data  = SPIWrp_SendAndReceiveByte(&hspi,p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txData);
        }
      }
    }while(p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.txOngoing);

  }
   
  /* Disable SPI peripheral */
  __HAL_SPI_DISABLE(&hspi);

}

/**
  * @brief      Send and receive byte through SPI
  * @param[in]  in_Metro_Device_Id (device ID), EXT1 to EXT4 and Data to send
  * @retval     Data recieved
  */

static uint8_t SPIWrp_SendAndReceiveByte(SPI_HandleTypeDef *hspi,uint8_t data)
{  
  uint8_t data_in;
  
  HAL_SPI_TransmitReceive(hspi,&data,&data_in,1,SPI_TIMEOUT);
  
  return(data_in);
  
}


#endif
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

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
