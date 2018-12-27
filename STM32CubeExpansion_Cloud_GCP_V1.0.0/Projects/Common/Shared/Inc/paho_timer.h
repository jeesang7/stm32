/**
  ******************************************************************************
  * @file    paho_timer.h
  * @author  MCD Application Team
  * @brief   Timer adaptation layer for the Paho MQTT client.
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
#ifndef PAHO_TIMER_H
#define PAHO_TIMER_H

/* Required by MQTTClient.h */

struct Timer {
  uint32_t init_tick;
  uint32_t timeout_ms;
};

typedef struct Timer Timer;

void TimerCountdownMS(Timer* timer, unsigned int timeout_ms);
void TimerCountdown(Timer* timer, unsigned int timeout);
int TimerLeftMS(Timer* timer);
char TimerIsExpired(Timer* timer);
void TimerInit(Timer* timer);

#endif  /* PAHO_TIMER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
