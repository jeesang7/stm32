/**
*   @file      metroTask.h
*   @author    STMicroelectronics
*   @version   V1.0
*   @date      11-March-2016
*   @brief     This code includes exported items for Metrology task
*   @note      (C) COPYRIGHT 2013 STMicroelectronics
*
* @attention
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
*/

#ifndef METROTASK_H
#define METROTASK_H

/*******************************************************************************
* INCLUDE FILES:
*******************************************************************************/
#include "stpm_metrology.h"
#include "metrology.h"

/** @addtogroup LEGAL
  * @{
  */

/*******************************************************************************
* CONSTANTS & MACROS:
*******************************************************************************/

#define METRO_PHASE_1           0
#define METRO_PHASE_2           1
#define METRO_PHASE_3           2
#define METRO_MAX_PHASES        3

#define METRO_DATA_ACTIVE       0
#define METRO_DATA_REACTIVE     1
#define METRO_DATA_APPARENT     2

/*******************************************************************************
* TYPES:
*******************************************************************************/
typedef struct {
  uint32_t       config;
  uint32_t       data1[19];
  uint32_t       powerFact[2];
  uint32_t       voltageFact[2];
  uint32_t       currentFact[2];
} nvmLeg_t;

typedef struct
{
  uint8_t       metroTimerActive;
  uint8_t       nbPhase;
  int32_t       powerActive;
  int32_t       powerReactive;
  int32_t       powerApparent;
  int32_t       energyActive;
  int32_t       energyReactive;
  int32_t       energyApparent;
  uint32_t      rmsvoltage;
  uint32_t      rmscurrent;
} metroData_t;

/*******************************************************************************
* GLOBAL VARIABLES:
*******************************************************************************/
extern metroData_t metroData;

/*******************************************************************************
* FUNCTIONS:
*******************************************************************************/
void METRO_Init();
void METRO_Update_Measures();
void METRO_Latch_Measures();

/**
  * @}
  */

#endif /* METROTASK_H */

/* End Of File */
