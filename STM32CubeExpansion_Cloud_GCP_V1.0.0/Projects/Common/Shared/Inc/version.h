/**
  ******************************************************************************
  * @file    version.h
  * @author  MCD Application Team
  * @brief   STM32 FW version definition
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
#ifndef version_H
#define version_H

#define FWVERSION_NAME_SIZE   20

/** Firmware version information.
 * @note  For user information.
 */
typedef struct {
  char name[FWVERSION_NAME_SIZE];
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  char *packaged_date;
} firmware_version_t;


#define FW_VERSION_MAJOR 1
#define FW_VERSION_MINOR 0
#define FW_VERSION_PATCH 0
#define FW_VERSION_DATE "14-September-2018 05:18:45 PM"

#ifdef AWS
#define FW_VERSION_NAME   "X-CUBE-AWS"
#define AWS_VERSION TBD
#endif

#ifdef AZURE
#define FW_VERSION_NAME   "X-CUBE-AZURE"
#define AZURE_VERSION TBD

#endif

#ifdef BLUEMIX
#define FW_VERSION_NAME   "X-CUBE-WATSON-X"
#define BLUEMIX_VERSION TBD

#endif

#if defined(GENERICMQTT) || defined(EXOSITEHTTP) || defined(HTTPCLIENT) || defined(GROVESTREAMS) || defined(UBIDOTS) || defined(WAKAAMACLIENT) || defined(NETTEST)
#define FW_VERSION_NAME   "X-CUBE-CLD-GEN"
#define LITMUS_VERSION TBD
#endif

#ifdef GOOGLEIOT
#define FW_VERSION_NAME   "X-CUBE-GCP"
#define GCPIOT_VERSION TBD
#endif

#endif /* version_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
