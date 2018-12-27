/**
  ******************************************************************************
  * @file    mbedtls_net.c
  * @author  MCD Application Team
  * @brief   Mbedtls network porting layer. Callbacks implementation.
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
#include "mbedtls_net.h"
#include "mbedtls/ssl.h"
#include "net.h"
#include "msg.h"
#include <string.h>

/* Non-blocking interface implementation. Timeout is not applicable. */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
  int ret = net_sock_recv((net_sockhnd_t) ctx, buf, len);
  
  if (ret > 0)
  {
    return ret;
  }
  
  switch(ret)
  {
    case 0:
      return  MBEDTLS_ERR_SSL_WANT_READ; 
    case NET_EOF:
      return 0;
    default:
    ;
  }
  
  msg_error("mbedtls_net_recv(): error %d in net_sock_recv() - requestedLen=%d\n", ret, len);
 
  return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}


/* Blocking interface implementation.*/
int mbedtls_net_recv_blocking(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
  int ret = 0;
  char stimeout[8];
  
  if ((sizeof(stimeout) - 1) <= snprintf(stimeout, sizeof(stimeout), "%lu",timeout))
  {
    msg_error("mbedtls_net_recv_blocking(): out of range timeout %lu\n", timeout);
    return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
  }
  
  if (net_sock_setopt((net_sockhnd_t) ctx, "sock_read_timeout", (uint8_t *)stimeout, strlen(stimeout) + 1) == NET_OK)
  {
    ret = net_sock_recv((net_sockhnd_t) ctx, buf, len);
  
    if (ret > 0)
    {
      return ret;
    }
    else
    {
      switch(ret)
      {
        case 0:
          return MBEDTLS_ERR_SSL_WANT_READ; 
        case NET_TIMEOUT:
          /* According to mbedtls headers, MBEDTLS_ERR_SSL_TIMEOUT should be returned. */
          /* But it saturates the error log with false errors. By contrast, MBEDTLS_ERR_SSL_WANT_READ does not raise any error. */
          return MBEDTLS_ERR_SSL_WANT_READ;
        case NET_EOF:
          return 0;
        default:
          ;
      }
    }
  }
  
  msg_error("mbedtls_net_recv_blocking(): error %d in net_sock_recv() - requestedLen=%d\n", ret, len);
  return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}


int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
  int rc = 0;
  int ret = net_sock_send((net_sockhnd_t) ctx, buf, len);

  if (ret > 0)
  {
    rc = ret;
  }
  else
  {
    switch(ret)
    {
      case 0:
        rc = MBEDTLS_ERR_SSL_WANT_WRITE;
        break;
      case NET_TIMEOUT:
        /* According to mbedtls headers, MBEDTLS_ERR_SSL_TIMEOUT should be returned. */
        /* But it saturates the error log with false errors. By contrast, MBEDTLS_ERR_SSL_WANT_WRITE does not raise any error. */
        rc = MBEDTLS_ERR_SSL_WANT_WRITE;
        break;
      case NET_EOF:
        rc = 0;
        break;
      default:
        rc = 0;
        msg_error("mbedtls_net_send(): error %d in net_sock_send() - requestedLen=%d\n", ret, len);
    }
  }

  /* Most often, errors are due to:
   *   - the closure of the connection by the remote host,
   *   - the disconnection of the network cable...
   */

  return rc;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
