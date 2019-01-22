/**
  ******************************************************************************
  * @file    stm3210e_discovery_lcdspi_ili9341.h
  * @author  MMS / MMY Team
  * @version V1.0.0
  * @date    15-october-2013
  * @brief   This file contains all the functions prototypes for the 
  *          stm3210e_discovery_lcdspi_ili9341.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM3210E_DISCOVERY_LCDSPI_ILI9341_H
#define __STM3210E_DISCOVERY_LCDSPI_ILI9341_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#if defined STM32F40_41xxx	 
	#include "stm32f4xx.h"
#else
	#include "stm32f10x.h"
#endif
	 
/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32F4_DISCOVERY
  * @{
  */ 

/** @addtogroup STM32F429I_DISCOVERY
  * @{
  */
    
/** @addtogroup STM32F429I_DISCOVERY_LCD
  * @{
  */ 


/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Types
  * @{
  */
typedef struct 
{
  int16_t X;
  int16_t Y;
} Point, * pPoint;   
/**
  * @}
  */ 

/** @defgroup stm32f429i_discovery_LCD_Exported_Constants
  * @{
  */ 

/* LCD Size (Width and Height) */
#define  LCD_PIXEL_WIDTH    ((uint16_t)320)
#define  LCD_PIXEL_HEIGHT   ((uint16_t)240)

#define LCD_FRAME_BUFFER       ((uint32_t)0xD0000000)
#define BUFFER_OFFSET          ((uint32_t)0x50000) 
/**
 * @brief Uncomment the line below if you want to use user defined Delay function
 *        (for precise timing), otherwise default _delay_ function defined within
 *         this driver is used (less precise timing).  
 */
/* #define USE_Delay */

#ifdef USE_Delay
#include "main.h" 
  #define _delay_     Delay  /* !< User can provide more timing precise _delay_ function
                                   (with 10ms time base), using SysTick for example */
#else
  #define _delay_     delay      /* !< Default _delay_ function with less precise timing */
#endif



/* LCD color */

#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Orange         0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

#define Line0          0
#define Line1          1
#define Line2          2
#define Line3          3
#define Line4          4
#define Line5          5
#define Line6          6
#define Line7          7
#define Line8          8
#define Line9          9

/** 
  * @brief  LCD Control pin  
  */ 
#define LCD_NCS_PIN             GPIO_Pin_12                  
#define LCD_NCS_GPIO_PORT       GPIOB                      
//#define LCD_NCS_GPIO_CLK        RCC_AHB1Periph_GPIOB 
#define LCD_NCS_GPIO_CLK        RCC_APB2Periph_GPIOB

/** 
  * @brief  LCD Command/data pin  
  */
// #define LCD_WRX_PIN             GPIO_Pin_12                  
//#define LCD_WRX_GPIO_PORT       GPIOE
//#define LCD_WRX_GPIO_CLK        RCC_AHB1Periph_GPIODE
//#define LCD_WRX_GPIO_CLK        RCC_APB2Periph_GPIOE

 #define LCD_WRX_PIN            GPIO_Pin_6                  
#define LCD_WRX_GPIO_PORT       GPIOC

#if defined STM32F40_41xxx
	#define LCD_WRX_GPIO_CLK        RCC_AHB1Periph_GPIOC
#else
	#define LCD_WRX_GPIO_CLK        RCC_APB2Periph_GPIOC
#endif

/** 
  * @brief  LCD SPI Interface pins 
  */ 
	
	/* LCD Control pins */

#define LCD_SPI_SCK_PIN               GPIO_Pin_7                     /* PF.07 */
#define LCD_SPI_SCK_GPIO_PORT         GPIOF                          /* GPIOF */
#define LCD_SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOF  
#define LCD_SPI_SCK_SOURCE            GPIO_PinSource7
#define LCD_SPI_SCK_AF                GPIO_AF_SPI5
#define LCD_SPI_MISO_PIN              GPIO_Pin_8                     /* PF.08 */
#define LCD_SPI_MISO_GPIO_PORT        GPIOF                          /* GPIOF */
#define LCD_SPI_MISO_GPIO_CLK         RCC_AHB1Periph_GPIOF  
#define LCD_SPI_MISO_SOURCE           GPIO_PinSource8
#define LCD_SPI_MISO_AF               GPIO_AF_SPI5
#define LCD_SPI_MOSI_PIN              GPIO_Pin_9                     /* PF.09 */
#define LCD_SPI_MOSI_GPIO_PORT        GPIOF                          /* GPIOF */
#define LCD_SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOF  
#define LCD_SPI_MOSI_SOURCE           GPIO_PinSource9
#define LCD_SPI_MOSI_AF               GPIO_AF_SPI5
//#define LCD_SPI                       SPI5
#define LCD_SPI                       SPI2
//#define LCD_SPI_CLK                   RCC_APB2Periph_SPI5 
#define LCD_SPI_CLK                   RCC_APB1Periph_SPI2

/** 
  * @brief  LCD Registers  
  */ 
#define LCD_SLEEP_OUT            0x11   /* Sleep out register */
#define LCD_GAMMA                0x26   /* Gamma register */
#define LCD_DISPLAY_OFF          0x28   /* Display off register */
#define LCD_DISPLAY_ON           0x29   /* Display on register */
#define LCD_COLUMN_ADDR          0x2A   /* Colomn address register */ 
#define LCD_PAGE_ADDR            0x2B   /* Page address register */ 
#define LCD_GRAM                 0x2C   /* GRAM register */   
#define LCD_MAC                  0x36   /* Memory Access Control register*/
#define LCD_PIXEL_FORMAT         0x3A   /* Pixel Format register */
#define LCD_WDB                  0x51   /* Write Brightness Display register */
#define LCD_WCD                  0x53   /* Write Control Display register*/
#define LCD_RGB_INTERFACE        0xB0   /* RGB Interface Signal Control */
#define LCD_FRC                  0xB1   /* Frame Rate Control register */
#define LCD_BPC                  0xB5   /* Blanking Porch Control register*/
#define LCD_DFC                  0xB6   /* Display Function Control register*/
#define LCD_POWER1               0xC0   /* Power Control 1 register */
#define LCD_POWER2               0xC1   /* Power Control 2 register */
#define LCD_VCOM1                0xC5   /* VCOM Control 1 register */
#define LCD_VCOM2                0xC7   /* VCOM Control 2 register */
#define LCD_POWERA               0xCB   /* Power control A register */
#define LCD_POWERB               0xCF   /* Power control B register */
#define LCD_PGAMMA               0xE0   /* Positive Gamma Correction register*/
#define LCD_NGAMMA               0xE1   /* Negative Gamma Correction register*/
#define LCD_DTCA                 0xE8   /* Driver timing control A */
#define LCD_DTCB                 0xEA   /* Driver timing control B */
#define LCD_POWER_SEQ            0xED   /* Power on sequence register */
#define LCD_3GAMMA_EN            0xF2   /* 3 Gamma enable register */
#define LCD_INTERFACE            0xF6   /* Interface control register */
#define LCD_PRC                  0xF7   /* Pump ratio control register */

/** 
  * @brief  LCD color  
  */ 
#define LCD_COLOR_WHITE          0xFFFF
#define LCD_COLOR_BLACK          0x0000
#define LCD_COLOR_GREY           0xF7DE
#define LCD_COLOR_BLUE           0x001F
#define LCD_COLOR_BLUE2          0x051F
#define LCD_COLOR_RED            0xF800
#define LCD_COLOR_MAGENTA        0xF81F
#define LCD_COLOR_GREEN          0x07E0
#define LCD_COLOR_CYAN           0x7FFF
#define LCD_COLOR_YELLOW         0xFFE0

/** 
  * @brief  LCD Direction  
  */ 
#define LCD_DIR_HORIZONTAL       0x0000
#define LCD_DIR_VERTICAL         0x0001

/**
  * @}
  */ 

/** 
  * @brief  LCD Layer  
  */ 
#define LCD_BACKGROUND_LAYER     0x0000
#define LCD_FOREGROUND_LAYER     0x0001

/**
  * @}
  */ 

/** @defgroup STM3210E_EVAL_LCD_Exported_Macros
  * @{
  */ 
#define ASSEMBLE_RGB(R, G, B)    ((((R)& 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))  

/**
  * @}
  */ 

/** @defgroup STM3210E_EVAL_LCD_Exported_Functions
  * @{
  */ 
void     LCD_DeInit(void);   
//void     LCD_Init(void);
void 		 LCD_Init(void);
void     LCD_LayerInit(void);
void     LCD_ChipSelect(FunctionalState NewState);
//void     LCD_ChipSelect(uint8_t NewState);
void     LCD_SetLayer(uint32_t Layerx);
void     LCD_SetColors(uint16_t _TextColor, uint16_t _BackColor); 
void     LCD_GetColors(uint16_t *_TextColor,uint16_t *_BackColor);
void     LCD_SetTextColor(uint16_t Color);
void     LCD_SetBackColor(uint16_t Color);
void     LCD_SetTransparency(uint8_t transparency);
void     LCD_ClearLine(uint16_t Line);
void     LCD_Clear(uint16_t Color);
void 		 LCD_SetCursor(uint16_t Xpos, u16 Ypos);
void     LCD_SetColorKeying(uint32_t RGBValue);
void     LCD_ReSetColorKeying(void);
void     LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c);
void     LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii);
void     LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr);
void     LCD_DisplayStringLineWSize(uint16_t Line, uint8_t *ptr, uint8_t size);
void     LCD_WindowModeDisable(void);
void     LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction);
void     LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void     LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     LCD_DrawEllipse(int Xpos, int Ypos, int Radius, int Radius2);
void     LCD_DrawFullEllipse(int Xpos, int Ypos, int Radius, int Radius2);
void     LCD_DrawMonoPict(const uint32_t *Pict);
void     LCD_WriteBMP(uint32_t BmpAddress);
void     LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void     LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     LCD_PolyLine(pPoint Points, uint16_t PointCount);
void     LCD_PolyLineRelative(pPoint Points, uint16_t PointCount);
void     LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount);
void     LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount);
void     LCD_FillPolyLine(pPoint Points, uint16_t PointCount);
void     LCD_Triangle(pPoint Points, uint16_t PointCount);
void     LCD_FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);
void     LCD_WriteCommand(uint8_t LCD_Reg);
void     LCD_WriteData(uint16_t value);
void     LCD_PowerOn(void);
void     LCD_DisplayOn(void);
void     LCD_DisplayOff(void);
void     LCD_CtrlLinesConfig(void);
void     LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal);
//void     LCD_CtrlLinesWrite(uint16_t * GPIOx, uint16_t CtrlPins, uint16_t BitVal);
void     LCD_SPIConfig(void);
void		 LCD_CtrlLinesConfig_ILI9341(void);
void 		 LCD_DisplayRandomPic(void);
void 		 LCD_nCS_StartByte(void);
void 	   LCD_WriteRAM_Prepare(void);
void     LCD_WriteRAM(u16 RGB_Code);
void		 LCD_WriteRAM_Close(void);
int 		 LCD_SetDisplayWindow(u16 Xpos, u16 Ypos, u16 Width , u16 Height );
void 		 LCD_WindowModeDisable(void);
void     LCD_WriteBMP_FAT(uint16_t Xpos, uint16_t Ypos, const char* BmpName);

/**
  * @}
  */    
  
#ifdef __cplusplus
}
#endif

#endif /* __STM3210E_DISCOVERY_LCDSPI_ILI9341_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
