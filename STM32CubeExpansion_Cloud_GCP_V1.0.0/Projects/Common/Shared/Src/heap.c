/**
  ******************************************************************************
  * @file    heap.c
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

#ifdef HEAP_DEBUG
#include "heap.h"
#include "stdlib.h"
#include "msg.h"

#ifdef HAS_RTOS
#include "FreeRTOS.h"
#endif

#ifndef CODE_UNDER_FIREWALL
#include "stdio.h"
#define heaplog      msg_info
#define MAGIC_NUMBER 0xDEADFA56
#else
#define MAGIC_NUMBER 0x12345678
/* msg_info is not working under firewall code */
#define heaplog(...)
#endif

#include "string.h"
static  uint32_t heap_allocated=0;
static  uint8_t *pmax=0;
static  uint8_t *pmin =(uint8_t*) 0xffffffff;

static size_t stack_max_size=0;
static unsigned char *stack_start;

#if defined (__ICCARM__)
extern void *CSTACK$$Base;
static unsigned char *stack_init = (unsigned char*) &CSTACK$$Base;
void __iar_dlfree(void *);
void * __iar_dlcalloc(size_t, size_t);
void __iar_dlmalloc_stats(void);
#elif defined ( __CC_ARM   )
extern void *__initial_sp;
static unsigned char *stack_init = (unsigned char*) &__initial_sp;
#elif defined (__GNUC__)
extern uint32_t _Min_Stack_Size;
extern uint32_t _estack;
static unsigned char *stack_init;
extern uint32_t get_heap_allocated(void);	/* See syscalls.c */
#endif

void stack_measure_prologue(void);
void stack_measure_epilogue(void);
void * STM32_malloc(size_t size);
void STM32_free(void* ptr);
void heap_info(void);

/*
* NB: Keil: Prevent stdlib heap usage: http://www.keil.com/support/man/docs/armlib/armlib_chr1358938927854.htm

*/

void heap_info()
{
#ifdef __ICCARM__ /* IAR: See https://www.iar.com/support/tech-notes/general/iar-dlib-library-heap-usage-statistics/ */
  __iar_dlmalloc_stats();
#elif defined (__CC_ARM ) /* Keil / armcc: See http://www.keil.com/support/man/docs/armlib/armlib_chr1359122851244.htm */
	/* /!\ Not available in MicroLib. Only in RealView CLib */
	#error "HEAP_DEBUG is not fully ported to MDK-ARM (Keil)"
	/*
  typedef int (*__heapprt)(void *, char const *, ...);
  __heapstats( (__heapprt)fprintf, stdout );
	*/
#elif defined (__GNUC__) /* GNU compiler */
  printf("LibC heap: Low-level allocated size: %lu\n", get_heap_allocated());
#endif

#ifdef HAS_RTOS
  printf("FreeRTOS heap: Low-level allocated size: %u\n", configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize());
#endif
}


void stack_measure_prologue()
{
  unsigned char p;
  stack_start = &p;

#if defined (__GNUC__) && !defined ( __CC_ARM   )
  stack_init = ((unsigned char*) &_estack) - ((uint32_t) &_Min_Stack_Size);
#endif

  memset(stack_init, 0xC8, stack_start-stack_init-10);
}


void stack_measure_epilogue()
{
  unsigned char * p = stack_init;
  while(*p == 0xC8) p++;
  size_t stack_size = stack_start - p;
  if (stack_size > stack_max_size)
  {
    stack_max_size=stack_size;
  }
}


void heap_stat(uint32_t *heap_max,uint32_t *heap_current,uint32_t *stack_size)
{
  *heap_max = pmax-pmin;
  *heap_current = heap_allocated;
  *stack_size = stack_max_size;
  heap_info();
}


// Overloading in IAR: --redirect malloc=STM32_malloc --redirect free=STM32_free --redirect calloc=heap_alloc
void * STM32_malloc(size_t size)
{
  return heap_alloc(size, 1);
}


void STM32_free(void * ptr)
{
  heap_free(ptr);
}


static  void heap_abort(void)
{
  heaplog("HEAP is corrupted \n");
  while(1);
}


void *heap_alloc(size_t a,size_t b)
{
  uint8_t *p;
  unsigned int n = a*b;

  n = (n + 3) & ~3; /* ensure alignment for magic number */
  n += 8;           /* add 2 x 32 bit for size and magic number */

#ifdef __ICCARM__ /* IAR */
  p = (uint8_t*)__iar_dlcalloc(n,1);  /* calloc may be redirected as well through the --redirect linker directive. */
#elif defined (__CC_ARM ) /* Keil / armcc */
  p = (uint8_t*)calloc(n,1);
#elif defined (__GNUC__) /* GNU compiler */
  p = (uint8_t*)calloc(n,1);
#endif
  
  if (p == NULL)
  {
    heaplog("HEAP run out of memory\n");
    while(1);
    /* return 0;*/
  }
  if ((p+n) > pmax) pmax = (p+n);
  if (p < pmin) pmin = p;

  heap_allocated += n;

  *((uint32_t*)p) = n; /* remember size */
  *((uint32_t*)(p - 4 + n)) = MAGIC_NUMBER; /* write magic number  after user allocation */

  return p+4;
}


void heap_free(void  *p)
{
  if (p==0) return;

  p = (uint8_t*)p - 4;
  int n = *((uint32_t*)p);

  heap_allocated-=n;

  /* check that magic number is not corrupted */
  if (*((uint32_t*)((uint8_t*) p + n - 4)) != MAGIC_NUMBER)
  {
    heap_abort(); /* error: data overflow or freeing already freed memory */
  }
  /* remove magic number to be able to detect freeing already freed memory */
  *((uint32_t*)((uint8_t*) p + n -4)) = 0;

#ifdef __ICCARM__ /* IAR */
  __iar_dlfree(p);
#elif defined (__CC_ARM ) /* Keil / armcc */
  free(p);
#elif defined (__GNUC__) /* GNU compiler */
  free(p);
#endif
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
