/**
  ******************************************************************************
  * @file    iot_flash_config.h
  * @author  CLAB Team
  * @brief   Header for JWT
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
#ifndef JWT_H
#define JWT_H

/** Function return codes. */
#define  JWT_OK                  0   /**< Success */
#define  JWT_ERROR              -1   /**< Internal error */
#define  JWT_KEY_ERROR          -2   /**< Key format error */
#define  JWT_MD_ERROR           -3   /**< HASH error */
#define  JWT_SIGN_ERROR         -4   /**< Sign error */

/* External interface ---------------------------------------------------------------*/

/**
 * @brief   Create a JSON Web Token, as specified in IoT Core of Google Cloud Platform.
 * @param   Out:  token       pointer to the memory where created JWT will be stored.
 * @param   In:   token_len   Max length of the token buffer.
 * @param   In:   date        the current date.
 * @param   In:   project_id  the GCP Project Id.
 * @param   In:   project_id  pointer to the device private key (RS256 or ES256) for creating JWT in pem format.
 * @param   In:   keylen      the private key length.
 * @retval  Status
 *            NET_OK      Success.
 *            NET_PARAM   The specified interface is not supported.
 *            NET_ERR     Error.
 */
int create_jwt(unsigned char *token, int token_len, int date, char *projectid, const unsigned char *key, int keylen);

#endif /* JWT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
