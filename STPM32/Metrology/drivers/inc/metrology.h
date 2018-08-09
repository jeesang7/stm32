/**
  ******************************************************************************
  * @file    metrology.h
  * @author  STMicroelectronics
  * @version V1.0
  * @date    11-March-2016
  * @brief   This file contains all the functions prototypes for the Generic Metrology
  *          firmware library, module metrology.c.
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __METROLOGY_H
#define __METROLOGY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------------------------*/
/* --------------------------------------------  Includes -----------------------------------------------------------*/
/* ------------------------------------------------------------------------------------------------------------------*/


#include "stpm_metrology.h"
   
/* ------------------------------------------------------------------------------------------------------------------*/
/* -----------------------------------------  Exported types --------------------------------------------------------*/
/* ------------------------------------------------------------------------------------------------------------------*/


 /**
  * @brief METROLOGY Reset type
  *
  */  
   
typedef enum 
{
  RESET_SYN_SCS = 1,
  RESET_SW
}METRO_ResetType_t;  


 /**
  * @brief METROLOGY  Current CHANNEL definition
  *
  */  
   
typedef enum 
{
  PRIMARY = 0,  
  SECONDARY,
  ALGEBRIC, 
  SIGMA_DELTA  
}METRO_LED_Channel_t; 


 /**
  * @brief METROLOGY  LED Slection type
  *
  */  
   
typedef enum 
{
  LED1 = 1,  
  LED2 
}METRO_LED_Selection_t; 


/**
  * @brief METROLOGY  Power selection type
  *
  */  
   
typedef enum 
{
  W_ACTIVE = 1,  
  F_ACTIVE,
  REACTIVE, 
  APPARENT_RMS,
  APPARENT_VEC,
  MOM_WIDE_ACT,
  MOM_FUND_ACT
}METRO_Power_selection_t;

typedef enum 
{
  LED_W_ACTIVE = 0,  
  LED_F_ACTIVE,
  LED_REACTIVE, 
  LED_APPARENT_RMS,

}METRO_LED_Power_selection_t;


typedef enum 
{
  E_W_ACTIVE = 1,  
  E_F_ACTIVE,
  E_REACTIVE, 
  E_APPARENT,
  NB_MAX_TYPE_NRJ
}METRO_Energy_selection_t;


/**
  * @brief METROLOGY  Latch device type
  *
  */  
typedef enum 
{
  LATCH_SYN_SCS = 1,  
  LATCH_SW,
  LATCH_AUTO
 }METRO_Latch_Device_Type_t;

  
 /**
  * @brief METROLOGY  Boolean  type
  *
  */  
typedef enum 
{
  BOOL_FALSE = 0,
  BOOL_TRUE  
}METRO_Bool_Type_t;
  
 /**
  * @brief METROLOGY External device Number
  *
  */  
   
typedef enum 
{
  EXT1=0,  
  NB_MAX_DEVICE,
  HOST,  
}METRO_NB_Device_t;  

 /**
  * @brief METROLOGY  CHANNEL definition
  *
  */  
   
typedef enum 
{
  CHANNEL_NONE=0,
  CHANNEL_1,  
  CHANNEL_2,
  NB_MAX_CHANNEL  
}METRO_Channel_t; 

typedef enum 
{
  INT_NONE_CHANNEL=0,
  INT_CHANNEL_1,  
  INT_CHANNEL_2,
  CHANNEL_TAMPER
}METRO_internal_Channel_t; 



 /**
  * @brief METROLOGY hardware Device type
  *
  */
     
typedef enum 
{
  Device_NONE=0,
  STM32,
  STPM32,                           
  STPM33,                            
  STPM34,
}METRO_Device_t;


/* Struct to define communication between STM32 and STPMs chips */
typedef struct
{
  uint8_t            rxData;
  uint8_t            txData;
  uint8_t            txValid;
  uint8_t            rxValid;
  uint8_t            txOngoing;
  uint8_t            rxOngoing;  
  uint8_t            *pTxReadBuf;
  uint8_t            *pTxWriteBuf;
  uint8_t            *pRxReadBuf;
  uint8_t            *pRxWriteBuf;
} STPM_Com_t;

/**
  * @brief METROLOGY Mapping Channels ( 1 to 4 ) to real V and C chip channels 
  * according to the Device
  *  Put NONE_CHANNEL if the channel is not mapped oterhwise  CHANNEL_1, CHANNEL_2, CHANNEL_3, CHANNEL_4   */


typedef struct
{
  METRO_Device_t              device;      
  uint8_t                          channels_mask;
  uint32_t                         factor_power_int_ch1;
  uint32_t                         factor_energy_int_ch1;
  uint32_t                         factor_power_int_ch2;
  uint32_t                         factor_energy_int_ch2;
  uint32_t                         factor_voltage_int_ch1;
  uint32_t                         factor_current_int_ch1;
  uint32_t                         factor_voltage_int_ch2;
  uint32_t                         factor_current_int_ch2;
  METRO_Latch_Device_Type_t   latch_device_type;  
  STPM_Com_t                  STPM_com;
  METRO_STPM_TypeDef          metro_stpm_reg;
}METRO_Device_Config_t;



typedef struct
{
  int32_t       energy[NB_MAX_CHANNEL][NB_MAX_TYPE_NRJ];
  int32_t       energy_extension[NB_MAX_CHANNEL][NB_MAX_TYPE_NRJ];
}METRO_Data_Energy_t;


#define    CHANNEL_MASK_CONF_CHANNEL_1     0x01
#define    CHANNEL_MASK_CONF_CHANNEL_2     0x02
#define    CHANNEL_MASK_CONF_CHANNEL_3     0x04
#define    CHANNEL_MASK_CONF_CHANNEL_4     0x08
  
#define    NB_NAX_CHANNEL                   3
 

/* ------------------------------------------------------------------------------------------------------------------*/
/* -------------------------------------  Exported functions --------------------------------------------------------*/
/* ------------------------------------------------------------------------------------------------------------------*/

/****************/
/* Device Level */
/****************/

/* Initialization and Setup functions *********************************/
void Metro_Init(void);
void Metro_power_up_device(void);
void Metro_UartSpeed(uint32_t baudrate);
void Metro_Config_Reset(METRO_ResetType_t in_MetroResetType);
void Metro_Set_Hardware_Factors(METRO_Channel_t in_Metro_Channel, uint32_t in_Factor_Power,uint32_t in_Factor_Nrj,uint32_t in_Factor_Voltage,uint32_t in_Factor_Current);

/* set metrology Config */
uint8_t Metro_Setup(uint32_t in_stpm_config);

/* Get setup Metrology */
uint8_t Metro_Get_Setup(uint32_t * out_p_stpm_config);

uint8_t Metro_ApplyConfig(uint32_t in_stpm_config, uint32_t in_stpm_data);
/* Set / Get Latch the device registers according to the latch type selection driving SYN pin  */
/* or setting auto-latch by S/W Auto Latch bit */
/* Latch_Type : SYN_SCS, SW, AUTO */

uint8_t Metro_Set_Latch_device_type(METRO_NB_Device_t in_Metro_Device, METRO_Latch_Device_Type_t in_Metro_Latch_Device_Type);
uint8_t Metro_Register_Latch_device_Config_type(METRO_NB_Device_t in_Metro_Device, METRO_Latch_Device_Type_t in_Metro_Latch_Device_Type);

/* Read energy */
/* in_Metro_energy_Selection : W_ACTIVE , F_ACTIVE, REACTIVE, APPARENT */
int32_t Metro_Read_energy(METRO_Channel_t in_Metro_Channel,METRO_Energy_selection_t in_Metro_Energy_Selection);

/* Read Power */
/* in_Metro_Power_Selection : W_ACTIVE , F_ACTIVE, REACTIVE, APPARENT_RMS, APPARENT_VEC, MOM_WIDE_ACT, MOM_FUND_ACT */
int32_t Metro_Read_Power(METRO_Channel_t in_Metro_Channel,METRO_Power_selection_t in_Metro_Power_Selection);

/* Read RMS */
/* in_RAW_vs_RMS : 0 : Raw values from registers requestest at output, 1 : RMS values in mV or mA requested at output */
void Metro_Read_RMS(METRO_Channel_t in_Metro_Channel,uint32_t * out_Metro_RMS_voltage,uint32_t * out_Metro_RMS_current, uint8_t in_RAW_vs_RMS);

/* Read / Write data block from Device ( Reg access to External STPM from UART/SPI */
/*********************************************************************************************************/
uint8_t Metro_Read_Block_From_Device ( METRO_NB_Device_t in_Metro_Device_Id, uint8_t in_Metro_Device_Offset_Adress, uint8_t in_Metro_Nb_of_32b_Reg, uint32_t *p_buffer );
uint8_t Metro_Write_Block_to_Device ( METRO_NB_Device_t in_Metro_Device_Id, uint8_t in_Metro_Device_Offset_Adress, uint8_t in_Metro_Nb_of_32b_Reg, uint32_t *in_p_buffer );



#ifdef __cplusplus
}
#endif

#endif /* __METROLOGY_H */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
