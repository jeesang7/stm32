/**
  ******************************************************************************
  * @file    rfu.h
  * @author  MCD Application Team
  * @brief   Remote firmware update over TCP/IP.
  *          Header for rfu.c file
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
#ifndef rfu_H
#define rfu_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#define RFU_OK                0
#define RFU_ERR               -1
#define RFU_ERR_HTTP          -2  /**< HTTP error */
#define RFU_ERR_FLASH         -3  /**< FLASH erase or programming error */
#define RFU_ERR_HTTP_CLOSED   -4  /**< The HTTP connection was closed by the server. */

/**
 * @brief   Download a firmware image from an HTTP server into Flash memory.
 * @note    The HTTP server must support the "Range:" request header. This is the case with HTTP/1.1.
 * @param   In: url       Network location of the new firmware (HTTP URL: "http://<hostname>:<port>/<path>")
 * @param   In: ca_certs  Root CA certificates (required for server authentication if a TLS session is needed)
 * @retval  Error code
 *             RFU_OK (0) Success.
 *             <0         Failure.
 *                          RFU_ERR_HTTP  Error downloading over HTTP.
 *                          RFU_ERR_FLASH Error erasing or programming the Flash memory.
 */
int rfu_update(const char * const url, const char * ca_certs);

#ifdef __cplusplus
}
#endif

#endif /* rfu_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
