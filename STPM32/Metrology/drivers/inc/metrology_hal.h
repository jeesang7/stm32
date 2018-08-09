/**
  ******************************************************************************
  * @file    metrology_hal.h
  * @author  STMicroelectronics
  * @version V1.0
  * @date    11-March-2016
  * @brief   This file contains all the functions prototypes for the Metrology
  *          firmware library, module comet_metrology.c.
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
#ifndef __METROLOGY_HAL_H
#define __METROLOGY_HAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/

#define   STPM_WAIT     1
#define   STPM_NO_WAIT  2


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

/* Device functions */
void Metro_HAL_init_device(METRO_NB_Device_t in_Metro_Device_Id);
void Metro_HAL_reset_device(METRO_ResetType_t in_MetroResetType,METRO_NB_Device_t in_Metro_Device_Id);
uint8_t Metro_HAL_Setup(METRO_Device_Config_t * in_p_Metro_Config);
void Metro_HAL_Set_Latch_device_type(METRO_NB_Device_t in_Metro_Device_Id, METRO_Latch_Device_Type_t in_Metro_Latch_Device_Type);
void Metro_HAL_power_up_device(METRO_NB_Device_t in_Metro_Device_Id);

/* Read measure functions */
int32_t Metro_HAL_read_power(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel,METRO_Power_selection_t in_Metro_Power_Selection);
int64_t Metro_HAL_read_energy(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel,METRO_Energy_selection_t in_Metro_Energy_Selection);
uint32_t Metro_HAL_read_RMS_Voltage(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel);
uint32_t Metro_HAL_read_RMS_Current(METRO_NB_Device_t in_Metro_Device_Id,METRO_internal_Channel_t in_Metro_int_Channel);

/* Elemntary access to STPM */
uint32_t Metro_HAL_Stpm_Read(METRO_NB_Device_t in_Metro_Device_Id,uint8_t * in_p_data,uint8_t nb_blocks,uint32_t * out_p_read_data);
uint32_t Metro_HAL_Stpm_write(METRO_NB_Device_t in_Metro_Device_Id,uint8_t * in_p_data,uint8_t nb_blocks,uint32_t * in_p_Buffer,uint8_t in_wait_stpm);
uint8_t Metrology_HAL_ReadBlock(METRO_NB_Device_t in_Metro_Device_Id, uint8_t Offset, uint8_t BlockNum, uint32_t * out_p_Buffer);
uint8_t Metrology_HAL_WriteBlock(METRO_NB_Device_t in_Metro_Device_Id, uint8_t Offset, uint8_t BlockNum, uint32_t * in_p_Buffer);

uint8_t Metro_HAL_baudrate_set(METRO_NB_Device_t in_Metro_Device_Id,uint32_t in_baudrate);


#ifdef __cplusplus
}
#endif

#endif /* __METROLOGY_HAL_H */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
