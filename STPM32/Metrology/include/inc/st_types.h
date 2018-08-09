/**
*   @file      st_types.h
*   @author    IPC - Industrial BU
*   @date      15 september 2013
*   @brief     This file includes general type re-definitions
*   @note      (C) COPYRIGHT 2013 STMicroelectronics
*
* @attention
*
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
*/

#ifndef ST_TYPES_H
#define ST_TYPES_H

/*******************************************************************************
* INCLUDE FILES:
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/** @addtogroup GENERIC
  * @{
  */

/*******************************************************************************
* CONSTANTS & MACROS:
*******************************************************************************/
#if !defined( __MODULE__ )
/* define Armcc __MODULE__ C Ansi __FILE__ */
#define __MODULE__ __FILE__
#endif

#define SL_STATIC_INLINE static __forceinline
#define __ALIGN(n)        __align(n)

#define MAX_VALUE(a,b) (((a) > (b)) ? (a) : (b))
#define MIN_VALUE(a,b) (((a) < (b)) ? (a) : (b))
#define ABS_VALUE(a)   (((a) < 0) ? (-(a)) : (a))
#define OFFSET_OF(s,f) (Uint32)&(((s *)0)->f)

//  #define __ASM            __asm                                      /*!< asm keyword for IAR Compiler          */
//  #define __INLINE                                    /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
//  #define __STATIC_INLINE 
#define INLINE inline                                    /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define __THROW 


/*******************************************************************************
* TYPES:
*******************************************************************************/
typedef u8 uint8_t;
typedef s8 int8_t;
typedef u16 uint16_t;
typedef s16 int16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;
typedef s32 int32_t;
typedef s64 int64_;
typedef bool boolean;

/* Callback function type: */
typedef void (CallBackFunctionType)(void);

/* Generic Callback function type (with a general purpose 32 bits parameter) */
typedef void (GenericCallBackFunctionType)(u32 data);

/*******************************************************************************
* GLOBAL VARIABLES:
*******************************************************************************/

/*******************************************************************************
* FUNCTIONS:
*******************************************************************************/

#endif /* ST_TYPES_H */

/** 
  * @}
  */

/* End Of File */
