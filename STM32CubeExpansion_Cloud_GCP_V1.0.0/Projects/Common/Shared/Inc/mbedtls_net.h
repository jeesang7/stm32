/**
  ******************************************************************************
  * @file    mbedtls_net.h
  * @author  MCD Application Team
  * @brief   MbedTLS network callbacks declaration.
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
#ifndef MBEDTLS_NET_H
#define MBEDTLS_NET_H

#include "mbedtls/ssl.h"  /* Some return codes are defined in ssl.h, so that the functions below may be used as mbedtls callbacks. */

int  mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len);
int  mbedtls_net_recv_blocking(void *ctx, unsigned char *buf, size_t len, uint32_t timeout);
int  mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len);

#endif /* MBEDTLS_NET_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
