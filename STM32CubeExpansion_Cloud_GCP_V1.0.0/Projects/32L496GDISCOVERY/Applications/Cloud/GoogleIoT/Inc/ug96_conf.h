/**
  ******************************************************************************
  * @file    ug96_conf.h
  * @author  MCD Application Team
  * @brief   UG96 configuration file.
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

#ifndef UG96_CONF_H
#define UG96_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* --------------------------------------------------------------------------- */
/* UG96 static parameter that can be configured by the user before compilation */
/* --------------------------------------------------------------------------- */

/* Max sockets allowed by modem is 12 but better use max 9 (sw not tested above) */
#define UG96_MAX_SOCKETS                        1
/* Max context allowed by modem is 20 but better use max 9 (sw not tested above) */
#define UG96_MAX_CONTEXTS                       1

/* 256 is normally sufficient, but if some params like URL are very long string it can be increased*/
#define UG96_CMD_SIZE                           256

/* Ug96 and STM32 shall have same baudrate to communicate */
#define UG96_DEFAULT_BAUDRATE                   115200

/* Rx and Tx buffer size, depend as the applic handles the buffer */
#define UG96_TX_DATABUF_SIZE                    1460 /* mbedTLS needs at least 1460 */
#define UG96_RX_DATABUF_SIZE                    1500

/* 0 to disable, 1 to enable PING feature */
#define UG96_USE_PING                           1

#ifdef __cplusplus
}
#endif
#endif /* UG96_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
