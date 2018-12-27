/**
  ******************************************************************************
  * @file    paho_timer.c
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
 
/**
 * @file timer.c
 * @brief Linux implementation of the timer interface.
 */
#include "main.h"
#include "paho_timer.h"  


void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
  timer->init_tick = HAL_GetTick();
  timer->timeout_ms = timeout_ms;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
  TimerCountdownMS(timer, timeout * 1000);
}


int TimerLeftMS(Timer* timer)
{
  int ret = 0;
  uint32_t cur_tick = HAL_GetTick();  /* The HAL tick period is 1 millisecond. */
  if (cur_tick < timer->init_tick)
  { /* Timer wrap-around detected */
    /* printf("Timer: wrap-around detected from %d to %d\n", timer->init_tick, cur_tick); */
    timer->timeout_ms -= 0xFFFFFFFF - timer->init_tick;
    timer->init_tick = 0;
  }
  ret = timer->timeout_ms - (cur_tick - timer->init_tick);

  return (ret >= 0) ? ret : 0;
}


char TimerIsExpired(Timer* timer)
{
  return (TimerLeftMS(timer) > 0) ? 0 : 1;
}


void TimerInit(Timer* timer)
{
  timer->init_tick = 0;
  timer->timeout_ms = 0;
}

