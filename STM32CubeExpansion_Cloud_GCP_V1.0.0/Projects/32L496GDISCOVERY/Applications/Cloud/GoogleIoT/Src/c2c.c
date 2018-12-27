/**
  ******************************************************************************
  * @file    c2c.c
  * @author  MCD Application Team
  * @brief   C2C interface file.
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
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "c2c.h"
#include <stdlib.h>
#include <stdbool.h>

/* Private define ------------------------------------------------------------*/
#define C2C_CONTEXT_1   1

/* Private global variables ---------------------------------------------------------*/
Ug96Object_t Ug96C2cObj;
static uint16_t rnglocalport = 0;

/* Private functions ---------------------------------------------------------*/
void C2C_ConvertIpAddrToString(const uint8_t* IpArray, char* ReturnString);


void C2C_ConvertIpAddrToString(const uint8_t* IpArray, char* ReturnString)
{
  snprintf((char*)ReturnString, 17, "%d.%d.%d.%d", IpArray[0], IpArray[1], IpArray[2], IpArray[3]);
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Initialize the LowLayer part of the C2C core
  * @param  registration_timeout_sec:
  * - value either positive (blocking with timeout) or max (blocking infinite)
  * @retval Operation status
  */
C2C_RegiStatus_t C2C_Init(uint16_t registration_timeout_sec)
{
  UG96_InitRet_t  init_status;
  UG96_SIMState_t sim_tmp;
  C2C_RegiStatus_t ret = C2C_REGISTATUS_ERROR;
  bool signalMessage = false;

  /* Channel signal quality */
  int32_t quality_level = 0;
  int8_t quality_level_db = 0;

  /* counter for PS attachment */
  int8_t attRetry = 0;

  /* Init for timeout management*/
  uint32_t tickstart;
  uint32_t tickcurrent;
  uint32_t registration_timeout_msec = registration_timeout_sec*1000;

  tickstart = HAL_GetTick();

  UG96_RegisterTickCb(&Ug96C2cObj, HAL_GetTick);

  if(UG96_RegisterBusIO(&Ug96C2cObj,
                           UART_C2C_Init,
                           UART_C2C_DeInit,
                           UART_C2C_SetBaudrate,
                           UART_C2C_SendData,
                           UART_C2C_ReceiveSingleData,
                           UART_C2C_FlushBuffer) == UG96_RETURN_OK)
  {

    init_status =  UG96_Init(&Ug96C2cObj);

    if(init_status == UG96_INIT_RET_OK)
    {
      tickcurrent = HAL_GetTick() - tickstart;
      while((tickcurrent <  registration_timeout_msec) || (registration_timeout_sec == C2C_MAX_DELAY))
      {
        /* check signal */
        if(UG96_GetSignalQualityStatus(&Ug96C2cObj, &quality_level) == UG96_RETURN_OK)
        {
          if (quality_level == 99)
          {
            if (signalMessage == false)
            {
              printf("Signal not known or not detectable yet (be patient) ");
              signalMessage = true;
            }
            else
            {
              printf(".");
            }
            HAL_Delay(1000);
          }
          else
          {
            quality_level_db = (int8_t) (-113 + 2*quality_level);
            printf("\nSignal Level: %d dBm \n", quality_level_db);
            HAL_Delay(5000);
            break;
          }
        }
        tickcurrent = HAL_GetTick() - tickstart;
      }

      tickcurrent = HAL_GetTick() - tickstart;
      while((tickcurrent <  registration_timeout_msec) || (registration_timeout_sec == C2C_MAX_DELAY))
      {
        /* attach the MT to the packet domain service */
        if(UG96_PSAttach(&Ug96C2cObj) == UG96_RETURN_OK)
        {
          printf("Packet Switched attachment succeeded\n");
          break;
        }
        else
        {
          /* start an automatic PLMN selection */
          attRetry++;
          if (attRetry == 1)
          {
            printf("Trying an automatic registration. It may take until 3 minutes, please wait ...\n");
            if (UG96_RETURN_OK != UG96_AutomaticPlmnSelection(&Ug96C2cObj))
              break;
          }
          else
          {
            if (attRetry > 4)
            {
              printf("Unrecoverable Error, PS attachment failed\n");
              break;
            }
          }
          HAL_Delay(1000);
        }
        tickcurrent = HAL_GetTick() - tickstart;
      }

      /* Check Packet Switched Registration */
      ret = (C2C_RegiStatus_t) UG96_GetPsNetworkRegistrationStatus(&Ug96C2cObj);
      if ((ret == C2C_REGISTATUS_HOME_NETWORK) || (ret == C2C_REGISTATUS_ROAMING))
      {
        tickcurrent = HAL_GetTick() - tickstart;
        printf("Registration done in %lu msseconds\n", tickcurrent);
      }
    }
    else
    {
      if(init_status == UG96_INIT_RET_SIM_ERR)
      {
        sim_tmp = Ug96C2cObj.SimInfo.SimStatus;
        if (sim_tmp == UG96_OPERATION_NOT_ALLOW)
        {
          ret = C2C_REGISTATUS_SIM_OP_NOT_ALLOWED;
        }
        else
        {
          ret = (C2C_RegiStatus_t) Ug96C2cObj.SimInfo.SimStatus;
        }
      }
      else
      {
        ret = C2C_REGISTATUS_ERROR; /* generic e.g. module does not respond to AT command */
      }
    }
  }
  return ret;
}

/**
  * @brief  List a defined number of available operators (modem needs a minute to reply, don't use it)
  * @param  Qvalue: Quality Level
  * @retval Operation status
  */
C2C_Ret_t C2C_GetSignalQualityStatus(int32_t *Qvalue)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_GetSignalQualityStatus(&Ug96C2cObj, Qvalue) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  List a defined number of available operators (modem needs a minute to reply, don't use it)
  * @param  Operators: pointer to string
  * @retval Operation status
  */
C2C_Ret_t C2C_ListOperators(char *Operators)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_ListOperators(&Ug96C2cObj, Operators) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Get current Operator
  * @param  Operators: pointer to string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetCurrentOperator(char *Operator, int32_t Bufsize)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_GetCurrentOperator(&Ug96C2cObj, Operator, Bufsize) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Force registration to specific Network Operator (by operator code).
  * @param  OperatorCode: http://www.imei.info/operator-codes/
  * @retval Operation Status.
  */
C2C_Ret_t C2C_ForceOperator(int32_t OperatorCode)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_ForceOperator(&Ug96C2cObj, OperatorCode) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Configure a TCP/UDP Access Point
  * @param  ContextType : 1: IPV4
  * @param  Apn : access point name
  * @param  Username : Got IP Address
  * @param  Password : Network IP mask
  * @param  AuthenticationMode : 0: none, 1: PAP
  * @retval Operation status
  */
C2C_Ret_t C2C_ConfigureAP(uint8_t ContextType,
                          const char* Apn,
                          const char* Username,
                          const char* Password,
                          uint8_t AuthenticationMode)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  UG96_APConfig_t ApConfig;

  ApConfig.ContextID = C2C_CONTEXT_1;
  strncpy((char*)ApConfig.ApnString, (char*)Apn, UG96_MAX_APN_NAME_SIZE);
  strncpy((char*)ApConfig.Username, (char*)Username, UG96_MAX_USER_NAME_SIZE);
  strncpy((char*)ApConfig.Password, (char*)Password, UG96_MAX_PSW_NAME_SIZE);
  ApConfig.Authentication = (UG96_Authent_t) AuthenticationMode;


  if(UG96_ConfigureAP(&Ug96C2cObj, &ApConfig) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Join a PDP Access Point
  * @retval Operation status
  */
C2C_Ret_t C2C_Connect(void)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_Activate(&Ug96C2cObj, C2C_CONTEXT_1) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Leave a PDP Access Point
  * @retval Operation status
  */
C2C_Ret_t C2C_Disconnect(void)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_Deactivate(&Ug96C2cObj, C2C_CONTEXT_1) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Check PDP Access Point state
  * @retval Access Point status
  */
C2C_APState_t C2C_IsConnected(void)
{
  return (C2C_APState_t) UG96_IsActivated(&Ug96C2cObj, C2C_CONTEXT_1);
}

/**
  * @brief  List the IP addresses for each context configured/activated
  * @param  IPaddr_string: pointer where to retrieve the string with all active IP info
  * @param  IPaddr_int: pointer where to retrieve the first active IP address in int_array[] format
  * @retval Operation status
  */
C2C_Ret_t C2C_GetActiveIpAddresses(char *IPaddr_string, uint8_t* IPaddr_int)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_GetActiveIpAddresses(&Ug96C2cObj, IPaddr_string, IPaddr_int) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Ping an IP address in the network
  * @param  host_addr_string: domain name (e.g. www.amazon.com) or dotted decimal IP address
  * @param  count: PING repetitions (default 4) (max 10)
  * @param  timeout_sec: timeout for each repetition in seconds
  * @retval Operation status
  */
C2C_Ret_t C2C_Ping(char *host_addr_string, uint16_t count, uint16_t timeout_sec)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_Ping(&Ug96C2cObj, C2C_CONTEXT_1, host_addr_string, count, timeout_sec) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  for a given host name it inquires the DNS service to get the IP address
  * @param  host_addr_string: host domain name
  * @param  ipaddr : pointer where to store the retrieved IP addr in uint8_t[4] format
  * @retval Operation status
  */
C2C_Ret_t C2C_GetHostAddress(const char *host_addr_string, uint8_t *ipaddr)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  /* This blocking call may take several seconds before returning */
  if(UG96_DNS_LookUp(&Ug96C2cObj, C2C_CONTEXT_1, host_addr_string, ipaddr) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Configure and start a client connection
  * @param  socket : Connection ID
  * @param  type : Connection type TCP/UDP
  * @param  host_url : name of the connection (e.g. www.amazon.com)
  * @param  ipaddr : IP addr in array numbers uint8_t[4] (just used if host_url == NULL)
  * @param  port : Remote port
  * @param  local_port : Local port
  * @retval Operation status
  */
C2C_Ret_t C2C_StartClientConnection(uint32_t socket, C2C_Protocol_t type, const char* host_url, uint8_t* ipaddr, uint16_t port, uint16_t local_port)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  char converted_ipaddr[17];
  UG96_Conn_t conn;
  int random_number = 0;

  conn.ConnectID = socket;
  conn.RemotePort = port;
#ifdef ACCESS_MODE_DIRECT_PUSH
  conn.AccessMode = UG96_DIRECT_PUSH; /* not fully tested */
#else
  conn.AccessMode = UG96_BUFFER_MODE;
#endif /* ACCESS_MODE_DIRECT_PUSH */

  if (local_port != 0)
  {
    conn.LocalPort = local_port;
  }
  else
  {
    /* The IANA range for ephemeral ports is 49152-65535. */
    /* implement automatic nr by sw because Quectel assigns always the same initial nr */
    /* generate random local port  number between 49152 and 65535 */
    if (rnglocalport == 0)  /* just at first open since board reboot */
    {
      random_number = rand();
      rnglocalport = ((uint16_t) (random_number & 0xFFFF) >> 2) + 49152;
    }
    else /* from second time function execution, increment by one */
    {
      rnglocalport += 1;
    }

    if (rnglocalport < 49152) /* Wrap-around */
    {
      rnglocalport = 49152;
    }

    conn.LocalPort = rnglocalport;
  }

  switch (type) {
    case C2C_TCP_PROTOCOL:
      conn.Type = UG96_TCP_CONNECTION;
      break;
    case C2C_UDP_SERVICE_PROTOCOL:
      conn.Type = UG96_UDP_SERVER_CONNECTION;
      break;
    default:
      ret = C2C_RET_NOT_SUPPORTED;
  }

  if (ret != C2C_RET_NOT_SUPPORTED)
  {
    if(host_url == NULL)
    {
      C2C_ConvertIpAddrToString(ipaddr, converted_ipaddr);
      conn.Url = converted_ipaddr;
    }
    else
    {
      conn.Url = (char *)host_url;
    }
    if(UG96_OpenClientConnection(&Ug96C2cObj, C2C_CONTEXT_1, &conn) == UG96_RETURN_OK)
    {
      ret = C2C_RET_OK;
    }
  }
  return ret;
}

/**
  * @brief  Configure and close a client connection
  * @param  socket : Connection ID
  * @retval Operation status
  */
C2C_Ret_t C2C_StopClientConnection(uint32_t socket)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  UG96_Conn_t conn;

  conn.ConnectID = socket;
  if(UG96_CloseClientConnection(&Ug96C2cObj, &conn) == UG96_RETURN_OK)
  {
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Configure and start a server connection
  * @param  socket : Connection ID
  * @param  type : Connection type TCP_LISTENER/UDP_SERVICE
  * @param  port : Remote port
  * @param  local_port : Local port
  * @retval Operation status
  */
C2C_Ret_t C2C_StartServerConnection(uint32_t socket, C2C_Protocol_t type, uint16_t port, uint16_t local_port)
{
  return C2C_RET_NOT_SUPPORTED;
}

/**
  * @brief  Configure and close a server connection
  * @param  socket : Connection ID
  * @retval Operation status
  */
C2C_Ret_t C2C_StopServerConnection(uint32_t socket)
{
  return C2C_RET_NOT_SUPPORTED;
}

/**
  * @brief  Send Data on a socket
  * @param  pdata : pointer to data to be sent
  * @param  IN: Reqlen : length of data to be sent
  * @param  OUT: SentDatalen : Length actually sent
  * @param  Timeout : time (ms) for the modem to confirm the data was sent. hence function could take longer to return.
  * @retval Operation status
  */
C2C_SendStatus_t C2C_SendData(uint32_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *SentDatalen, uint32_t Timeout)
{
  C2C_SendStatus_t ret = C2C_SEND_ERROR;
  UG96_SendRet_t status;

  status = UG96_SendData(&Ug96C2cObj, socket, pdata, Reqlen, SentDatalen, Timeout);
  if( status == UG96_SEND_RET_SENT)
  {
    ret = C2C_SEND_OK;
  }
  if( status == UG96_SEND_RET_BUF_FULL)
  {
    ret = C2C_SEND_BUF_FULL;
  }

  return ret;
}

/**
  * @brief  Send Data on a socket
  * @param  pdata : pointer to data to be sent
  * @param  Reqlen : length of data to be sent
  * @param  OUT: SentDatalen : length actually sent
  * @param  Timeout : Socket write timeout (ms)
  * @param  IN: ipaddr : 4-byte array containing the IP address of the remote host
  * @param  IN: port : port number of the remote host
  * @retval Operation status
  */
C2C_SendStatus_t C2C_SendDataTo(uint32_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *SentDatalen, uint32_t Timeout, uint8_t *ipaddr, uint16_t port)
{
  C2C_SendStatus_t ret = C2C_SEND_ERROR;
  UG96_SendRet_t status;

  status = UG96_SendDataTo(&Ug96C2cObj, socket, pdata, Reqlen, SentDatalen, Timeout, ipaddr, port);
  if( status == UG96_SEND_RET_SENT)
  {
    ret = C2C_SEND_OK;
  }

  return ret;
}


/**
  * @brief  Receive Data from a socket
  * @param  pdata : pointer to Rx buffer
  * @param  IN: Reqlen : in UG96_BUFFER_MODE the req len, in UG96_DIRECT_PUSH is the max leng available in pdata[] buffer
  * @param  OUT: *RcvDatalen :  pointer to length of data
  * @param  Timeout : timeout (ms) used by each internal exchange MCU<-->modem; hence function could take much longer to return
  * @retval Operation status
  */
C2C_Ret_t C2C_ReceiveData(uint32_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *RcvDatalen, uint32_t Timeout)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  UG96_ReceiveRet_t result;

  result = UG96_ReceiveData(&Ug96C2cObj, socket, pdata, Reqlen, RcvDatalen, Timeout);
  if( (result == UG96_RECEIVE_RET_OK) || (result == UG96_RECEIVE_RET_INCOMPLETE))
  {
    ret = C2C_RET_OK;
  }

  return ret;
}

/**
  * @brief  Receive Data from a socket
  * @param  pdata : pointer to Rx buffer
  * @param  Reqlen : maximum length of the data to be received
  * @param  OUT: RcvDatalen : length of the data actually received
  * @param  Timeout : Socket read timeout (ms)
  * @param  OUT: ipaddr : 4-byte array containing the IP address of the remote host
  * @param  OUT: port : port number of the remote host
  * @retval Operation status
  */
C2C_Ret_t C2C_ReceiveDataFrom(uint32_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *RcvDatalen, uint32_t Timeout, uint8_t *ipaddr, uint16_t *port)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  UG96_ReceiveRet_t result;

  result = UG96_ReceiveDataFrom(&Ug96C2cObj, socket, pdata, Reqlen, RcvDatalen, Timeout, ipaddr, port);
  if(result == UG96_RECEIVE_RET_OK)
  {
    ret = C2C_RET_OK;
  }

  return ret;
}


/**
  * @brief  Retrieve last error details from C2C modem
  * @param  ErrorString: pointer to string
  * @retval Operation status
  */
C2C_Ret_t C2C_RetrieveLastErrorDetails(char *ErrorString)
{
  C2C_Ret_t ret = C2C_RET_ERROR;
  char errstring[UG96_ERROR_STRING_SIZE];

  memset(errstring, 0, UG96_ERROR_STRING_SIZE);

  if(UG96_RetrieveLastErrorDetails(&Ug96C2cObj, errstring) == UG96_RETURN_OK)
  {
    strncpy(ErrorString, errstring, MIN(C2C_ERROR_STRING, UG96_ERROR_STRING_SIZE));
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Reset the C2C module
  * @retval Operation status
  */
void C2C_ResetModule(void)
{
  C2C_HwResetAndPowerUp();
}

/**
  * @brief  Power Off the C2C module
  */
void C2C_Power_Off(void)
{
  UG96_PowerDown(&Ug96C2cObj);
}

/**
  * @brief  Restore module default configuration
  * @retval Operation status
  */
C2C_Ret_t C2C_SetModuleDefault(void)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(UG96_ResetToFactoryDefault(&Ug96C2cObj) == UG96_RETURN_OK)
  {
      ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Update module firmware
  * @param  location : Binary Location IP address
  * @retval Operation status
  */
C2C_Ret_t C2C_ModuleFirmwareUpdate(const char *location)
{
  return C2C_RET_NOT_SUPPORTED;
}

/**
  * @brief  Return Module firmware revision
  * @param  rev : revision string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetModuleFwRevision(char *rev)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(Ug96C2cObj.FW_Rev != NULL)
  {
    strncpy(rev, (char *)Ug96C2cObj.FW_Rev, MIN(C2C_FW_REV_SIZE, UG96_FW_REV_SIZE));
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Return Product ID
  * @param  ProductId : Product ID string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetModuleID(char *ProductId)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(Ug96C2cObj.ProductID != NULL)
  {
    strncpy(ProductId, (char *)Ug96C2cObj.ProductID, MIN(C2C_PROD_ID_SIZE, UG96_PROD_ID_SIZE));
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Return Module Name
  * @param  Info : Module Name string
  * @retval Operation status
  */
C2C_Ret_t C2C_GetModuleName(char *ModuleName)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(Ug96C2cObj.Manufacturer != NULL)
  {
    strncpy(ModuleName, (char *)Ug96C2cObj.Manufacturer, MIN(C2C_MFC_SIZE, UG96_MFC_SIZE));
    ret = C2C_RET_OK;
  }
  return ret;
}

/**
  * @brief  Return Module Name
  * @param  SimId : returns the ICCID (Integrated Circuit Card Identifier) of the SIM card
  * @retval Operation status
  */
C2C_Ret_t C2C_GetSimId(char *SimId)
{
  C2C_Ret_t ret = C2C_RET_ERROR;

  if(Ug96C2cObj.Manufacturer != NULL)
  {
    strncpy(SimId, (char *)Ug96C2cObj.SimInfo.ICCID, MIN(C2C_ICCID_SIZE, UG96_ICCID_SIZE));
    ret = C2C_RET_OK;
  }
  return ret;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

