/**
  ******************************************************************************
  * @file    paho_mqtt_platform.h
  * @author  MCD Application Team
  * @brief   IoT Discovery Kit Paho MQTT client porting layer.
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
#ifndef PAHO_MQTT_PLATFORM_H
#define PAHO_MQTT_PLATFORM_H
#include "stdint.h"
#include "stdint.h"

#include "paho_timer.h"  /* Provides the timer interface required by Paho. */

struct Network
{
  int32_t *my_socket;
  int (*mqttread) (struct Network*, unsigned char*, int, int);
  int (*mqttwrite) (struct Network*, unsigned char*, int, int);
};

typedef struct Network Network; 

#endif /* PAHO_MQTT_PLATFORM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
