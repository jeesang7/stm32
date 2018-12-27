/**
  ******************************************************************************
  * @file    heap.h
  * @author  MCD Application Team
  * @brief   Heap check functions.
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
#ifndef HEAP_H
#define HEAP_H

#ifdef HEAP_DEBUG
#include "stdint.h"
#include "stddef.h"

/* Stack instrumentation */
#ifndef HAS_RTOS
void stack_measure_prologue(void);
void stack_measure_epilogue(void);
#endif /* HAS_RTOS */

/* Heap instrumentation */
#if defined (__ICCARM__)
#define MEM_TRACE(msg) { uint32_t heap_max, heap_current, stack_size;\
                      heap_stat(&heap_max, &heap_current, &stack_size);\
                      printf("%s\n\theap_max: %lu, heap_cur: %lu\n", (msg), heap_max, heap_current); }
#else
#define MEM_TRACE(msg)	heap_info()
#endif

void *heap_alloc(size_t a,size_t b);  
void heap_free(void  *p);
void heap_info(void);
void heap_stat(uint32_t *heap_max,uint32_t *heap_current, uint32_t *stacksize);

#else /* HEAP_DEBUG */
#define MEM_TRACE(msg)

#endif /* HEAP_DEBUG */

#endif  /* HEAP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
