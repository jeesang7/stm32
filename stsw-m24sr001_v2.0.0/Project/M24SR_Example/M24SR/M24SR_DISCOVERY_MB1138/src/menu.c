/**
  ******************************************************************************
  * @file    menu.c 
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    08/21/2013
  * @brief   Menu navigation driver for M24SR-DISCOVERY demonstration.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup User_Appli
 * 	@{
 *  @brief      <b>This folder contains the application files</b> 
 */
 
/** @addtogroup Menu
 * 	@{
 *  @brief      This file contains the menu navigation driver for M24SR-DISCOVERY. 
 */


/* As there is lot's of picture the size of the code increase a lot */
/* and the time to load the application is longer... */
/* to speed up things you can define the DEBUG flag to embed less picture */
//#define DEBUG

/* bmp to be used by this application */
#ifndef DEBUG

/* logo for introduction */
#include "stlogo.h"

/* icon file for menu */
#include "URL_icon.h"
#include "SMS_icon.h"
#include "email_icon.h"
#include "vcard_icon.h"
#include "Bluetooth_icon.h"
#include "geoloc_icon.h"
#include "MyApp_icon.h"
#include "SlideShow_icon.h"
#include "RF_Disable.h"
#include "remote_control_icon.h"
#include "lock_icon.h"
#include "disclaimer_icon.h"

/* special picture for vCard screen */
#include "vcard.h"
#include "VcardCSL1.h"
#include "customer_support.h"

/* slides presentation */
#include "SlideA_1.h"
#include "SlideA_2.h"
#include "SlideB_1.h"
#include "SlideB_2.h"
#include "SlideC_1.h"
#include "SlideC_2.h"
#include "SlideD_1.h"
#include "SlideD_2.h"
#include "antenna1.h"
#include "antenna2.h"

#else

/* icons */
#include "URL_icon.h"

/* special picture for vCard screen */
#include "vcard.h"
#include "VcardCSL1.h"
#include "customer_support.h"

/* slides presentation */
const char uSlideA_1[]= {0};
const char uSlideA_2[]= {0};
const char uSlideB_1[]= {0};
const char uSlideB_2[]= {0};
const char uSlideC_1[]= {0};
const char uSlideC_2[]= {0};
const char uSlideD_1[]= {0};
const char uSlideD_2[]= {0};
const char uAntenna1[]= {0};
const char uAntenna2[]= {0};

#endif

static __IO uint32_t TimingDelay = 0;
sMediaInfo MediaFile;

/* Private typedef -----------------------------------------------------------*/
typedef void (* tMenuFunc)(void);
typedef struct sMenuItem * tMenuItem;
typedef struct sMenu * tMenu;

/* Private function prototypes -----------------------------------------------*/
static uint8_t ReadKey(void);
static uint8_t Event(void);

static void WaitUserAction(void);
static void WaitDebounce(void);
static void Char2Hex( char* MacAddr ,uint32_t size);
static void Hex2Char( u8* pDataIn, u16 NumberOfByte, char* pString );

static void Menu_Init(void);
static void IdleFunc(void);
static void DisplayMenu(void);
static void DisplayIcons(void);
static void ReturnFunc(void);
static uint32_t DelayJoyStick(__IO uint32_t nTime);


static void DisplayURI(sURI_Info *pURI);
static void DisplaySMS(sSMSInfo *pSMSStruct);
static void DisplayEmail(sEmailInfo *pEmailStruct);
static void DisplayVcard(sVcardInfo *pVcardStruct);
static void DisplayGeo(sGeoInfo *pGeoStruct);
static void DisplayMessage(char *pMessage);
static void DisplayErrorMessage(void);
static void DisplayMessageOnGoing(void);
static void DisplayDwLdPhoneAppMessage( void);
static void DisplayLockPassword(bool NewPassWordSet, u8* pPassword);
static void DisplayUnLockPassword(bool PassWordState);
static void DisplaylimitedAccess( bool ActionCompleted);
static void DisplayNormalMode( bool ActionCompleted);

static void AboutFunc(void);
static void ProductPres(void);
static void AntennaPres(void);
static void Display_Info(void);
static void ReadURI(void);
static void Load_URI_URL(void);
static void Load_URI_Tel(void);
static void ReadSMS(void);
static void WriteSMS(void);
static void ReadEmail(void);
static void WriteEmail(void);
static void ReadVcard(void);
static void WriteVcard(void);
static void Load_BT_NDEF(void);
static void Enable_BT_Pairing(void);
static void ReadGeo(void);
static void WriteGeo(void);
static void DWLoadPhoneApp(void);
static void LaunchDemoApp(void);
static void Enable_M24SR_RF(void);
static void Disable_M24SR_RF(void);
static void EnableWritePWD (void);
static void EnableReadPWD (void);
static void DisableWritePWD (void);
static void DisableReadPWD (void);
static void EnableReadOnlyMode (void);
static void DisableReadOnlyMode (void);
static void EnableWriteOnlyMode (void);
static void DisableWriteOnlyMode (void);
static void SuperUser (void);
static void Enable_GPO_RF(void);

/* Private define ------------------------------------------------------------*/
#define M24SR_SLIDE_PRESENTATION 8  /* 4 slides but divided in 2 peaces */

#define NumberOfIcons 12
#define NbIconeByLine 4
#define NbIconeByColumn 3

/* icon screen are displayed like this */
/* icon are 60x60 bmp */

/********************************/
/*															*/
/*															*/
/*															*/
/*  =====  =====  =====  =====  */
/*  = 1 =  = 2 =  = 3 =  = 4 =  */
/*  =====  =====  =====  =====  */
/*															*/
/*  =====  =====  =====  =====  */
/*  = 5 =  = 6 =  = 7 =  = 8 =  */
/*  =====  =====  =====  =====  */
/*															*/
/*  =====  =====  =====  =====  */
/*  = 9 =  = 10=  = 11=  = 12=  */
/*  =====  =====  =====  =====  */
/*															*/
/********************************/

/* put icon address in this array, null if nothing to display */
/* put menu position that match with the icon */
#ifndef DEBUG
uint32_t IconsAddr[12] [2] = {{(uint32_t)(&uURL), 0}  			, {(uint32_t)(&uSMS), 1}		       , {(uint32_t)(&uEmail), 2} , {(uint32_t)(&uVcard), 3}			,
														  {(uint32_t)(&uBT),4}  				, {(uint32_t)(&uGeo), 5} 		 			 , {(uint32_t)(&uMyApp), 6} ,	{(uint32_t)(&uSlideShow), 7}		,
															{(uint32_t)(&uRF_DIS),8}      , {(uint32_t)(&uRemoteControl), 9} , {(uint32_t)(&uLock), 10} , {(uint32_t)(&uDisclaimer),11}	}; 
#else
uint32_t IconsAddr[12] [2] = {{(uint32_t)(&uURL), 0}  			, {(uint32_t)(&uURL), 1}		       , {(uint32_t)(&uURL), 2} 	, {(uint32_t)(&uURL), 3}			,
														  {(uint32_t)(&uURL),4}  				, {(uint32_t)(&uURL), 5} 		 			 , {(uint32_t)(&uURL), 6}	  ,	{(uint32_t)(&uURL), 7}		,
															{(uint32_t)(&uURL),8}      		, {(uint32_t)(&uURL), 9} 					 , {(uint32_t)(&uURL), 10}  , {(uint32_t)(&uURL),11}	}; 
#endif
															
uint16_t IconRect[12][2] ={{31, 33}, {96, 33}, {161, 33}, {226, 33},
 													 {31, 98}, {96, 98}, {161, 98}, {226, 98},
 													 {31, 163}, {96, 163}, {161, 163}, {226, 163}};

const char ASCII [16][2] = { {0x30,0x00}, {0x31,0x01}, {0x32,0x02}, {0x33,0x03}, {0x34,0x04},  
														 {0x35,0x05}, {0x36,0x06}, {0x37,0x07}, {0x38,0x08}, {0x39,0x09}, 
														 {0x61,0x0A}, {0x62,0x0B}, {0x63,0x0C}, {0x64,0x0D}, {0x65,0x0E},
														 {0x66,0x0F}, 
													 };

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Field to fill for the URI demonstration */                                                                                                                                                      
char Protocol[] = {"http://www."};
char URI_Message[] = {"st.com/nfc-rfid"};

/* Field to fill for the SMS demonstration */
char PhoneNumber[] = {"+33612345678\0"};
char Message[] = {"This SMS was generated automatically by tapping your phone near M24SR\0"};
char Instruction[] = {"This is an example of NFC generated SMS using M24SR\0"};

/* Field to fill for the Email demonstration */
char EmailAdd[] = {"customer.service@st.com\0"};
char Subject[] = {"M24SR S/N 754FHFGJF46G329 WARRANTY\0"};
char EmailMessage[] = {"this is a demonstration to illustrate an automatic warranty activation email\0"};
char Information[] = {"automatic warranty\0"};
            
/* Field to fill for the Vcard demonstration */
char FirstName[] = {"CanIHelp You\0"};
char Title[] = {"Customer support\0"};
char Org[] = {"STMicroelectronics\0"};
char HomeAddress[] = {"\0"};
char WorkAddress[] = {"190 avenue Celestin Coq 13106 ROUSSET FRANCE\0"};
char HomeTel[] = {"\0"};
char WorkTel[] = {"\0"};
char CellTel[] = {"+33 6 12 34 56 78\0"};
char HomeEmail[] = {"\0"};
char WorkEmail[] = {"customer.service@st.com \0"};         
													 											 
u8 NDEF_BT_PAIRING[] = {
 0x00, 0x44, /* NLEN; NDEF length (68 byte long message) */
 0xD2, //MB=1b, ME=1b, CF=0b, SR=1b, IL=0b, TNF=010b 
 0x20, //Record Type Length: 32 octets
 0x21, //payload length: 33 octets;
 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2F, 0x76, 
 0x6E, 0x64, 0x2E, 0x62, 0x6C, 0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74, 0x68, 0x2E, 
 0x65, 0x70, 0x2E, 0x6F, 0x6F, 0x62, /* Record Type Name: application/vnd.blue 
 tooth.ep.oob */
 0x21, 0x00, //OOB optional data length: 33 octets
 0x36, 0x5E, 0x2F, 0x6F, 0x12, 0x00, //bluetooth device address: 
 //00:12:6F:2F:5E:36 (example address only)
 0x0D, //EIR Data Length: 13 octets 
 0x09, //EIR Data Type: Complete Local Name
 0x4D, 0x32, 0x34, 0x53, 0x52, 0x2D, 0x44, 0x69, 0x73, 0x63,0x6F, 0x76, 
 // Bluetooth Local Name: M24SR-Discov
 0x04, //EIR Data Length: 4 octets
 0x0D, //EIR Data Type: Class of device
 0x04, 0x04, 0x20, /* Class of Device: 
 0x20, Service Class=Audio
 0x04, Major Device Class=Audio/Video
 0x04, Minor Device Class=Wearable */
 //Headset Device *:
 0x05, //EIR Data Length: 5 octets
 0x03, //EIR Data type: 16-bit Service Class UUID list (complete)
 0x1E, 0x11, 0x0B, 0x11 /*16-bit Service Class UUID list (complete) 
 ;0x111E – HFP-HF, 0x011B – A2DP-SNK*/};


/* Field to fill for the Geolocation demonstration */
char Latitude[] = {"48.132974\0"};
char Longitude[] = {"-1.635912\0"};
char GeoInformation[] = {"M24SR-DISCOVERY firmware birth place\0"};

 
bool ReadPasswordActivated = false;
bool WritePasswordActivated = false;
bool ReadOnlyModeActivated = false;
bool WriteOnlyModeActivated = false;

bool MenuActivated = true;
 
uint8_t MenuItemIndex = 0, nMenuLevel = 0;
uint8_t ItemNumb[MAX_MENU_LEVELS];

tMenuItem psMenuItem, psCurrentMenuItem;
tMenu psPrevMenu[MAX_MENU_LEVELS];
tMenu psCurrentMenu;

struct sMenuItem
{
  uint8_t* pszTitle;
  tMenuFunc pfMenuFunc;
  tMenuFunc pfUpDownMenuFunc;
  tMenu psSubMenu;
};

struct sMenu
{
  uint8_t* pszTitle;
  tMenuItem psItems;
  uint8_t nItems;
};


const uint32_t SlideAddr[M24SR_SLIDE_PRESENTATION] = {(uint32_t)(&uSlideA_1), (uint32_t)(&uSlideA_2),
																											(uint32_t)(&uSlideB_1), (uint32_t)(&uSlideB_2),
																											(uint32_t)(&uSlideC_1), (uint32_t)(&uSlideC_2),
																											(uint32_t)(&uSlideD_1), (uint32_t)(&uSlideD_2)};                      

const uint32_t DelayTiming[14] = {2420, 2600, 800, 2400, 4725, 2100, 3300, 3500, 1600, 3300,
                        2420, 3400, 2275, 1670};


/*------------------------------ Menu level 4 -------------------------------*/
struct sMenuItem RWLockMenuItems[] = {{"Enable Read Password", EnableReadPWD, IdleFunc},
																			{"Enable Write Pwd    ", EnableWritePWD, IdleFunc},
																			{"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu RWLockMenu = {" M24SR Lock Feature ", RWLockMenuItems, countof(RWLockMenuItems)};


struct sMenuItem RWUnLockMenuItems[] = {{"  Disable Read Pwd  ", DisableReadPWD, IdleFunc},
																				{"  Disable Write Pwd ", DisableWritePWD, IdleFunc},
																				{"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu RWUnLockMenu = {"M24SR Unlock Feature", RWUnLockMenuItems, countof(RWUnLockMenuItems)};


struct sMenuItem RWLimitedMenuItems[] = {{"  Enable Read Only  ", EnableReadOnlyMode, IdleFunc},
																		   	 {"  Enable Write Only ", EnableWriteOnlyMode, IdleFunc},
																			   {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu RWLimitedMenu = {"  M24SR ST Feature  ", RWLimitedMenuItems, countof(RWLimitedMenuItems)};


struct sMenuItem RWUnLimitedMenuItems[] = {{"  Disable Read Only ", DisableReadOnlyMode, IdleFunc},
																				   {" Disable Write Only ", DisableWriteOnlyMode, IdleFunc},
																			     {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu RWUnLimitedMenu = {"  M24SR ST Feature  ", RWUnLimitedMenuItems, countof(RWUnLimitedMenuItems)};


/*------------------------------ Menu level 3 -------------------------------*/
struct sMenuItem LoadURIMenuItems[] = {{"     Store URL      ", Load_URI_URL, IdleFunc},
																			 {" Store Phone Number ", Load_URI_Tel, IdleFunc},
																			 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu LoadURIMenu = {" Select URI to Load ", LoadURIMenuItems, countof(LoadURIMenuItems)};


struct sMenuItem PasswordMenuItems[] = {{"  Enable Password   ", IdleFunc, IdleFunc, &RWLockMenu},
																				{"  Disable Password  ", IdleFunc, IdleFunc, &RWUnLockMenu},
																				{"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu PasswordMenu = {"Pwd Authentification", PasswordMenuItems, countof(PasswordMenuItems)};


struct sMenuItem LimitedAccessMenuItems[] = {{"   Access Limited   ", IdleFunc, IdleFunc, &RWLimitedMenu},
																						 {"   No Restriction   ", IdleFunc, IdleFunc, &RWUnLimitedMenu},
																				     {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu LimitedAccessMenu = {"Pwd Authentification", LimitedAccessMenuItems, countof(LimitedAccessMenuItems)};

/*------------------------------ Menu level 2 -------------------------------*/
																	 
struct sMenuItem URIMenuItems[] = {{" Store URI in M24SR ", IdleFunc, IdleFunc, &LoadURIMenu},
                                   {" Read URI from M24SR", ReadURI, IdleFunc},
                                   {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu URIMenu = {"        URI         ", URIMenuItems, countof(URIMenuItems)};


struct sMenuItem SMSMenuItems[] = {{" Store SMS in M24SR ", WriteSMS, IdleFunc},
																	 {" Read SMS from M24SR", ReadSMS, IdleFunc},
																	 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu SMSMenu = {"        SMS         ", SMSMenuItems, countof(SMSMenuItems)};


struct sMenuItem EmailMenuItems[] = {{"Store Email in M24SR", WriteEmail, IdleFunc},
                                     {"Rd Email from M24SR ", ReadEmail, IdleFunc},
																		 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu EmailMenu = {"       Email        ", EmailMenuItems, countof(EmailMenuItems)};


struct sMenuItem VcardMenuItems[] = {{"Store Vcard in M24SR", WriteVcard, IdleFunc},
                                     {"Rd Vcard from M24SR ", ReadVcard, IdleFunc},
																		 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu VcardMenu = {"       Vcard        ", VcardMenuItems, countof(VcardMenuItems)};


struct sMenuItem BTMenuItems[] = {{"Set BT NDEF in M24SR", Load_BT_NDEF, IdleFunc},
                                  {"  Song Information  ", Display_Info, IdleFunc},
																	{"  Pair New Device   ", Enable_BT_Pairing, IdleFunc},
																	{"       Return       ", ReturnFunc, IdleFunc}};															 
struct sMenu BTMenu = {"   Select Action    ", BTMenuItems, countof(BTMenuItems)};


struct sMenuItem GeoMenuItems[] = {{" Store Pos in M24SR ", WriteGeo, IdleFunc},
																	 {" Read Pos from M24SR", ReadGeo, IdleFunc},
																	 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu GeoMenu = {"    Geolocation      ", GeoMenuItems, countof(GeoMenuItems)};


struct sMenuItem PrivateAppMenuItems[] = {{" DownLoad Phone App ", DWLoadPhoneApp, IdleFunc},
																					{"  Launch Demo App   ", LaunchDemoApp, IdleFunc},
																					{"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu PrivateAppMenu = {"   My Private App    ", PrivateAppMenuItems, countof(PrivateAppMenuItems)};


struct sMenuItem M24SRPresMenuItems[] = {{" M24SR Presentation ", ProductPres, IdleFunc},
																				 {"   Antenna Matrix   ", AntennaPres, IdleFunc},
																				 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu M24SRPresMenu = {" M24SR presentation ", M24SRPresMenuItems, countof(M24SRPresMenuItems)};


struct sMenuItem RFDisableMenuItems[] = {{"     Enable RF      ", Enable_M24SR_RF, IdleFunc},
                                         {"     Disable RF     ", Disable_M24SR_RF, IdleFunc},
																				 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu RFDisableMenu = {"  M24SR RF on/off   ", RFDisableMenuItems, countof(RFDisableMenuItems)};


struct sMenuItem RFGPOMenuItems[] = {{" DownLoad Phone App ", DWLoadPhoneApp, IdleFunc},
																		 {"  GPO Control Demo  ", Enable_GPO_RF, IdleFunc},
																		 {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu RFGPOMenu = {"   GPO RF control   ", RFGPOMenuItems, countof(RFGPOMenuItems)};


struct sMenuItem LockMenuItems[] = {{" Pwd Authentication ", IdleFunc, IdleFunc, &PasswordMenu},
                                    {"Force Limited Access", IdleFunc, IdleFunc, &LimitedAccessMenu},
																	  {"   Super User Mode  ", SuperUser, IdleFunc},
																	  {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu LockMenu = {"M24SR Lock Selection", LockMenuItems, countof(LockMenuItems)};


struct sMenuItem AboutMenuItems[] = {{"       About        ", AboutFunc, IdleFunc},
                                     {"       Return       ", ReturnFunc, IdleFunc}};
struct sMenu AboutMenu = {"       About        ", AboutMenuItems, countof(AboutMenuItems)};


/*------------------------------ Menu level 1 -------------------------------*/
struct sMenuItem MainMenuItems[] = {
	{"        URI         ", IdleFunc, IdleFunc, &URIMenu},	
	{"        SMS         ", IdleFunc, IdleFunc, &SMSMenu},
  {"       Email        ", IdleFunc, IdleFunc, &EmailMenu},
	{"       Vcard        ", IdleFunc, IdleFunc, &VcardMenu},
	{"     BT pairing     ", IdleFunc, IdleFunc, &BTMenu},
	{"    Geolocation     ", IdleFunc, IdleFunc, &GeoMenu},
	{"   My Private App   ", IdleFunc, IdleFunc, &PrivateAppMenu},
	{" M24SR presentation ", IdleFunc, IdleFunc, &M24SRPresMenu},
	{"  M24SR RF on/off   ", IdleFunc, IdleFunc, &RFDisableMenu},	
	{"  RF GPO control    ", IdleFunc, IdleFunc, &RFGPOMenu},
	{" M24SR Password mode", IdleFunc, IdleFunc, &LockMenu},
	{"     Disclaimer     ", IdleFunc, IdleFunc, &AboutMenu}};
struct sMenu MainMenu = {"     Main menu      ", MainMenuItems, countof(MainMenuItems)};


/* At power ON the password are set to 0x00.....0x00 by the firmware */
uint8_t CurrentReadPassword[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t CurrentWritePassword[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

bool LedContext[4] = { false, false, false, false };
bool LedActivated[4] = { true, true, true, true };


/** @addtogroup Menu_Private_Functions
 * 	@{
 */


/**
  * @brief  Reads key from demoboard.
  * @param  None
  * @retval Return RIGHT, LEFT, SEL, UP, DOWN or NOKEY
  */
static uint8_t ReadKey(void)
{
  /* "right" key is pressed */
  if(!GPIO_ReadInputDataBit( J_RIGHT_PORT, J_RIGHT_PIN ))
  {
    while(GPIO_ReadInputDataBit( J_RIGHT_PORT, J_RIGHT_PIN ) == Bit_RESET)
    {
    } 
    return RIGHT; 
  }	
  /* "left" key is pressed */
  if(!GPIO_ReadInputDataBit( J_LEFT_PORT, J_LEFT_PIN ))
  {
    while(GPIO_ReadInputDataBit( J_LEFT_PORT, J_LEFT_PIN ) == Bit_RESET)
    {
    }
    return LEFT; 
  }
  /* "up" key is pressed */
  if(!GPIO_ReadInputDataBit( J_UP_PORT, J_UP_PIN ))
  {
    while(GPIO_ReadInputDataBit( J_UP_PORT, J_UP_PIN ) == Bit_RESET)
    {
    }
    return UP; 
  }
  /* "down" key is pressed */
  if(!GPIO_ReadInputDataBit( J_DOWN_PORT, J_DOWN_PIN ))
  {
    while(GPIO_ReadInputDataBit( J_DOWN_PORT, J_DOWN_PIN ) == Bit_RESET)
    {
    } 
    return DOWN; 
  }
  /* "sel" key is pressed */
  if(!GPIO_ReadInputDataBit( J_SEL_PORT, J_SEL_PIN ))
  {
    while(GPIO_ReadInputDataBit( J_SEL_PORT, J_SEL_PIN ) == Bit_RESET)
    {
    } 
    return SEL; 
  }
  /* No key is pressed */
  else 
  {
    return NOKEY;
  }
}

/**
  * @brief  Check what event occurs.
  * @param  None
  * @retval Return RF_EVENT or KEY_EVENT
  */
static uint8_t Event(void)
{
	u8 status = NO_EVENT;
	
	/* If phone app made access to update M24SR, GPO will goes low */
	if( GPIO_ReadInputDataBit(M24SR_GPO_PIN_PORT,M24SR_GPO_PIN) == Bit_RESET)
		status = RF_EVENT;
		
	/* check if key event, user want to stop so not important if he was updated M24SR */
	if(ReadKey()!= NOKEY)
		status = KEY_EVENT;
		
	return status;
}

/**
  * @brief  Wait user presses joystick.
  * @param  None
  * @retval None
  */
static void WaitUserAction(void)
{	
	MenuActivated = false;
	while(ReadKey() == NOKEY)
  {	
  }
	MenuActivated = true;
}

/**
  * @brief  Wait debounce key.
  * @param  None
  * @retval None
  */
static void WaitDebounce(void)
{
	while(ReadKey() != NOKEY)
  {
  } 
}

/**
  * @brief  convert a char to it's ASCII coding byte.
  * @param  MacAddr : pointer on the string to convert
	* @param  size : the size of the string
  * @retval None
  */
static void Char2Hex( char* MacAddr ,uint32_t size)
{
	char result [12];
	uint8_t i=0,k=0,l=0;
	
	for(k=0; k<size; k++)
	{
		for(i=0; i<16; i++)
		{
			if (MacAddr[k] == ASCII [i][0])
			{	
				result[l] = ASCII [i][1];
				l++;
				break;
			}
		}
	}
	
	/* convert char to hex and switch byte order to ease copy in NDEF file */
	for(i=0; i<6; i++)
	{
		MacAddr[5-i] = ((result[2*i]&0x0F)<<4)| (result[2*i+1]&0x0F);
	}
}

/**
  * @brief  convert a ASCII coding byte to it's representing char.
  * @param  pDataIn : pointer on the byte array to translate
	* @param  NumberOfByte : the size of the array
	* @param  pString : pointer on the string created
  * @retval None
  */
static void Hex2Char( u8* pDataIn, u16 NumberOfByte, char* pString )
{
	u8 data;
	uint8_t i=0;
	
	for(i=0; i<NumberOfByte; i++)
	{
		/* First char */
		data = (*pDataIn & 0xF0)>>4;
		if( data < 0x0A)
			*pString = data + 0x30;  /* ASCII offset for number */
		else
			*pString = data + 0x37; 	/* ASCII offset for letter */
	
		pString++;
	
		/* Second char */
		data = (*pDataIn & 0x0F);
		if( data < 0x0A)
			*pString = data + 0x30;  /* ASCII offset for number */
		else
			*pString = data + 0x37; 	/* ASCII offset for letter */
	
		pString++;
		pDataIn++;
	}
	
}

/**
  * @brief  Screen config to display URI information
  * @param  pURI : pointer on URI structure
  * @retval None
  */
static void DisplayURI(sURI_Info *pURI)
{
	u32 ProtocolOffset = 0, ContentOffset = 0, InformationOffset = 0;
		
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
	
  
	LCD_DisplayStringLine(Line0, (u8*)"         URI          ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	/* Set the Text Color */
  LCD_SetTextColor(Green);
	LCD_DisplayStringLine(Line1, "URI protocol:       ");	
	LCD_DisplayStringLine(Line4, "URI content:        ");
	LCD_DisplayStringLine(Line7, "URI information:    ");
	
  /* Set the Text Color */
  LCD_SetTextColor(White);
		
	while(ReadKey() == NOKEY)
  {
		
		if(strlen(pURI->protocol))
		{			
			if(strlen(pURI->protocol) > 20 && ProtocolOffset < strlen(pURI->protocol)-15)
			{
				LCD_DisplayStringLineWSize(Line2, (u8*)(pURI->protocol)+ProtocolOffset,(u8)(strlen(pURI->protocol))-ProtocolOffset);	
				ProtocolOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line2, (u8*)(pURI->protocol),(u8)(strlen(pURI->protocol)));	
				ProtocolOffset=0;
			}
		}
		
		
		if(strlen(pURI->URI_Message))
		{			
			if(strlen(pURI->URI_Message) > 20 && ContentOffset < strlen(pURI->URI_Message)-15)
			{
				LCD_DisplayStringLineWSize(Line5, (u8*)(pURI->URI_Message)+ContentOffset,(u8)(strlen(pURI->URI_Message))-ContentOffset);	
				ContentOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line5, (u8*)(pURI->URI_Message),(u8)(strlen(pURI->URI_Message)));	
				ContentOffset=0;
			}
		}
		
		
		if(strlen(pURI->Information))
		{			
			if(strlen(pURI->Information) > 20 && InformationOffset < strlen(pURI->Information)-15)
			{
				LCD_DisplayStringLineWSize(Line8, (u8*)(pURI->Information)+InformationOffset,(u8)(strlen(pURI->Information))-InformationOffset);	
				InformationOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line8, (u8*)(pURI->Information),(u8)(strlen(pURI->Information)));	
				InformationOffset=0;
			}
		}
		
	}

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Screen config to display SMS information
  * @param  pSMSStruct : pointer on SMS structure
  * @retval None
  */
static void DisplaySMS(sSMSInfo *pSMSStruct)
{
	u32 PhoneNumberOffset = 0, SMSMessageOffset = 0, InstructionOffset = 0;
		
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, (u8*)"        SMS         ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	/* Set the Text Color */
  LCD_SetTextColor(Green);
	LCD_DisplayStringLine(Line2, (u8*)"Phone Number:       ");	
	LCD_DisplayStringLine(Line5, (u8*)"SMS message:        ");
	LCD_DisplayStringLine(Line8, (u8*)"Instruction:        ");
	
  /* Set the Text Color */
  LCD_SetTextColor(White);
		
	while(ReadKey() == NOKEY)
  {
		
		if(strlen(pSMSStruct->PhoneNumber))
		{			
			if(strlen(pSMSStruct->PhoneNumber) > 20 && PhoneNumberOffset < strlen(pSMSStruct->PhoneNumber)-15)
			{
				LCD_DisplayStringLineWSize(Line3, (u8*)(pSMSStruct->PhoneNumber)+PhoneNumberOffset,(u8)(strlen(pSMSStruct->PhoneNumber))-PhoneNumberOffset);	
				PhoneNumberOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line3, (u8*)(pSMSStruct->PhoneNumber),(u8)(strlen(pSMSStruct->PhoneNumber)));	
				PhoneNumberOffset=0;
			}
		}
		
		
		if(strlen(pSMSStruct->Message))
		{			
			if(strlen(pSMSStruct->Message) > 20 && SMSMessageOffset < strlen(pSMSStruct->Message)-15)
			{
				LCD_DisplayStringLineWSize(Line6, (u8*)(pSMSStruct->Message)+SMSMessageOffset,(u8)(strlen(pSMSStruct->Message))-SMSMessageOffset);	
				SMSMessageOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line6, (u8*)(pSMSStruct->Message),(u8)(strlen(pSMSStruct->Message)));	
				SMSMessageOffset=0;
			}
		}
		
		
		if(strlen(pSMSStruct->Information))
		{			
			if(strlen(pSMSStruct->Information) > 20 && InstructionOffset < strlen(pSMSStruct->Information)-15)
			{
				LCD_DisplayStringLineWSize(Line9, (u8*)(pSMSStruct->Information)+InstructionOffset,(u8)(strlen(pSMSStruct->Information))-InstructionOffset);	
				InstructionOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line9, (u8*)(pSMSStruct->Information),(u8)(strlen(pSMSStruct->Information)));	
				InstructionOffset=0;
			}
		}
		
	}

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
} 

/**
  * @brief  Screen config to display Email information
  * @param  pEmailStruct : pointer on Email structure
  * @retval None
  */
static void DisplayEmail(sEmailInfo *pEmailStruct)
{
	u32 EmailAddOffset = 0, SubjectOffset = 0, EmailMessageOffset = 0;
		
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, (u8*)"        Email         ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	/* Set the Text Color */
  LCD_SetTextColor(Green);
	LCD_DisplayStringLine(Line1, (u8*)"To:                   ");	
	LCD_DisplayStringLine(Line4, (u8*)"Subject:              ");
	LCD_DisplayStringLine(Line7, (u8*)"Message:              ");
	
  /* Set the Text Color */
  LCD_SetTextColor(White);
		
	while(ReadKey() == NOKEY)
  {
		
		if(strlen(pEmailStruct->EmailAdd))
		{			
			if(strlen(pEmailStruct->EmailAdd) > 20 && EmailAddOffset < strlen(pEmailStruct->EmailAdd)-15)
			{
				LCD_DisplayStringLineWSize(Line2, (u8*)(pEmailStruct->EmailAdd)+EmailAddOffset,(u8)(strlen(pEmailStruct->EmailAdd))-EmailAddOffset);	
				EmailAddOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line2, (u8*)(pEmailStruct->EmailAdd),(u8)(strlen(pEmailStruct->EmailAdd)));	
				EmailAddOffset=0;
			}
		}
		
		
		if(strlen(pEmailStruct->Subject))
		{			
			if(strlen(pEmailStruct->Subject) > 20 && SubjectOffset < strlen(pEmailStruct->Subject)-15)
			{
				LCD_DisplayStringLineWSize(Line5, (u8*)(pEmailStruct->Subject)+SubjectOffset,(u8)(strlen(pEmailStruct->Subject))-SubjectOffset);	
				SubjectOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line5, (u8*)(pEmailStruct->Subject),(u8)(strlen(pEmailStruct->Subject)));	
				SubjectOffset=0;
			}
		}
		
		
		if(strlen(pEmailStruct->Message))
		{			
			if(strlen(pEmailStruct->Message) > 20 && EmailMessageOffset < strlen(pEmailStruct->Message)-15)
			{
				LCD_DisplayStringLineWSize(Line8, (u8*)(pEmailStruct->Message)+EmailMessageOffset,(u8)(strlen(pEmailStruct->Message))-EmailMessageOffset);	
				EmailMessageOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line8, (u8*)(pEmailStruct->Message),(u8)(strlen(pEmailStruct->Message)));	
				EmailMessageOffset=0;
			}
		}
		
	}

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
} 

/**
  * @brief  Screen config to display Vcard information
  * @param  pVcardStruct : pointer on Vcard structure
  * @retval None
  */
static void DisplayVcard(sVcardInfo *pVcardStruct)
{

	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, (u8*)"       VCARD        ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(Green);
	
	/* Vcard version 3.0 not supported so far */
	if( !memcmp(pVcardStruct->Version, VCARD_VERSION_3_0, VCARD_VERSION_3_0_SIZE))
	{
		LCD_DisplayStringLineWSize(Line2, "  Vcard Version 3.0 ",20);
		LCD_DisplayStringLineWSize(Line3, " not supported with ",20);
		LCD_DisplayStringLineWSize(Line3, "    this firmware   ",20);
		goto end;
	}	
	
	LCD_DisplayStringLineWSize(Line2, (u8*)(pVcardStruct->FirstName),(u8)(strlen(pVcardStruct->FirstName)));
	
	if( memcmp(pVcardStruct->FirstName, "NDEF is not a Vcard ", 20))
	{
		/* Set the Text Color */
		LCD_SetTextColor(White);
	
		LCD_DisplayStringLineWSize(Line4, (u8*)(pVcardStruct->Title),(u8)(strlen(pVcardStruct->Title)));
		LCD_DisplayStringLineWSize(Line5, (u8*)(pVcardStruct->Org),(u8)(strlen(pVcardStruct->Org)));
	
		
		if( strlen(pVcardStruct->CellTel) == 0)
		{
			
			if( strlen(pVcardStruct->WorkTel) == 0)
				/* Home tel */
				LCD_DisplayStringLineWSize(Line7, (u8*)(pVcardStruct->HomeTel),(u8)(strlen(pVcardStruct->HomeTel)));
			else		
				/* Work tel */
				LCD_DisplayStringLineWSize(Line7, (u8*)(pVcardStruct->WorkTel),(u8)(strlen(pVcardStruct->WorkTel)));				
		}
		else
			/* Cell tel */
			LCD_DisplayStringLineWSize(Line7, (u8*)(pVcardStruct->CellTel),(u8)(strlen(pVcardStruct->CellTel)));

	
		
		if( strlen(pVcardStruct->WorkAddress) == 0)
			LCD_DisplayStringLineWSize(Line8, (u8*)(pVcardStruct->HomeAddress),(u8)(strlen(pVcardStruct->HomeAddress)));
		else
			LCD_DisplayStringLineWSize(Line8, (u8*)(pVcardStruct->WorkAddress),(u8)(strlen(pVcardStruct->WorkAddress)));
		
		
		if( strlen(pVcardStruct->WorkEmail) == 0)
			LCD_DisplayStringLineWSize(Line9, (u8*)(pVcardStruct->HomeEmail),(u8)(strlen(pVcardStruct->HomeEmail)));
		else
			LCD_DisplayStringLineWSize(Line9, (u8*)(pVcardStruct->WorkEmail),(u8)(strlen(pVcardStruct->WorkEmail)));
	
		/* Add a default picture as we don't manage it */
		if( !memcmp(pVcardStruct->FirstName, FirstName, strlen(FirstName)))
			LCD_WriteBMP_FAT(260, 25, (const char*)(uCSL1));
		else
			LCD_WriteBMP_FAT(260, 25, (const char*)(uPortrait));
		
	}

end:	
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
} 

/**
  * @brief  Screen config to display Geolocation information
  * @param  pGeoStruct : pointer on Geolocation structure
  * @retval None
  */
static void DisplayGeo(sGeoInfo *pGeoStruct)
{
	u32 LatitudeOffset = 0, LongitudeOffset = 0, InformationOffset = 0;
		
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, (u8*)"    Geolocation     ");

	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	/* Set the Text Color */
  LCD_SetTextColor(Green);

	LCD_DisplayStringLine(Line2, (u8*)"Latitude:           ");	
	LCD_DisplayStringLine(Line5, (u8*)"Longitude:          ");
	LCD_DisplayStringLine(Line8, (u8*)"Information:        ");
	
  /* Set the Text Color */
  LCD_SetTextColor(White);
		
	while(ReadKey() == NOKEY)
  {
		
		if(strlen(pGeoStruct->Latitude))
		{			
			if(strlen(pGeoStruct->Latitude) > 20 && LatitudeOffset < strlen(pGeoStruct->Latitude)-15)
			{
				LCD_DisplayStringLineWSize(Line3, (u8*)(pGeoStruct->Latitude)+LatitudeOffset,(u8)(strlen(pGeoStruct->Latitude))-LatitudeOffset);	
				LatitudeOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line3, (u8*)(pGeoStruct->Latitude),(u8)(strlen(pGeoStruct->Latitude)));	
				LatitudeOffset=0;
			}
		}
		
		
		if(strlen(pGeoStruct->Longitude))
		{			
			if(strlen(pGeoStruct->Longitude) > 20 && LongitudeOffset < strlen(pGeoStruct->Longitude)-15)
			{
				LCD_DisplayStringLineWSize(Line6, (u8*)(pGeoStruct->Longitude)+LongitudeOffset,(u8)(strlen(pGeoStruct->Longitude))-LongitudeOffset);	
				LongitudeOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line6, (u8*)(pGeoStruct->Longitude),(u8)(strlen(pGeoStruct->Longitude)));	
				LongitudeOffset=0;
			}
		}
		
		
		if(strlen(pGeoStruct->Information))
		{			
			if(strlen(pGeoStruct->Information) > 20 && InformationOffset < strlen(pGeoStruct->Information)-15)
			{
				LCD_DisplayStringLineWSize(Line9, (u8*)(pGeoStruct->Information)+InformationOffset,(u8)(strlen(pGeoStruct->Information))-InformationOffset);	
				InformationOffset++;
			}
			else
			{
				LCD_DisplayStringLineWSize(Line9, (u8*)(pGeoStruct->Information),(u8)(strlen(pGeoStruct->Information)));	
				InformationOffset=0;
			}
		}
		
	}

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
} 


/**
  * @brief  Screen config to inform user that content has been stored in M24SR
  * @param  pMessage : pointer on message explaining which content has been stored
  * @retval None
  */
static void DisplayMessage(char *pMessage)
{
	/* Set the Title Back Color */
  LCD_SetBackColor(Black);

  /* Set the Title Color */
  LCD_SetTextColor(White);
	
	/* Clear the LCD Screen */
  LCD_Clear(Blue);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Text Back Color */
  LCD_SetBackColor(Blue);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLineWSize(Line2, (u8*)pMessage,(u8)(strlen(pMessage)));
	LCD_DisplayStringLine(Line3, "  stored in M24SR   ");
	LCD_DisplayStringLine(Line5, "    Waiting for     ");
	LCD_DisplayStringLine(Line6, "     NFC phone      ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
	/* External Interrupt Enable */
	IntExtOnOffConfig(ENABLE);
	
	/* wait user action to go back to previous screen */
	WaitUserAction();
	
	/* External Interrupt Disable */
	IntExtOnOffConfig(DISABLE);		
	
	/* Clear the LCD Screen */
  LCD_Clear(White);
	/* Display Menu */
  DisplayMenu();
}

/**
  * @brief  Screen config to warn user that content has not been stored in M24SR
  * @param  None
  * @retval None
  */
static void DisplayErrorMessage(void)
{
	/* Set the Title Back Color */
  LCD_SetBackColor(Black);

  /* Set the Title Color */
  LCD_SetTextColor(White);
	
	/* Clear the LCD Screen */
  LCD_Clear(Blue);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Text Back Color */
  LCD_SetBackColor(Blue);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line2, "  NDEF not stored   ");
	LCD_DisplayStringLine(Line3, "  Please check the  ");
	LCD_DisplayStringLine(Line4, "  NFC phone is not  ");
	LCD_DisplayStringLine(Line5, "   close to M24SR   ");
	LCD_DisplayStringLine(Line6, " antenna, leaving RF");
	LCD_DisplayStringLine(Line7, "   session opened   ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
	/* External Interrupt Enable */
	IntExtOnOffConfig(ENABLE);
	
	/* wait user action to go back to previous screen */
	WaitUserAction();
	
	/* External Interrupt Disable */
	IntExtOnOffConfig(DISABLE);		
	
	/* Clear the LCD Screen */
  LCD_Clear(White);
	/* Display Menu */
  DisplayMenu();
}

/**
  * @brief  Screen config to warn user that content is being stored in M24SR
  * @param  None
  * @retval None
  */
static void DisplayMessageOnGoing(void)
{
	/* Set the Title Back Color */
  LCD_SetBackColor(Black);

  /* Set the Title Color */
  LCD_SetTextColor(White);
	
	/* Clear the LCD Screen */
  LCD_Clear(Blue);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Text Back Color */
  LCD_SetBackColor(Blue);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line3, "    Storing 7kB     ");
	LCD_DisplayStringLine(Line4, "       Vcard        ");
	LCD_DisplayStringLine(Line5, "      in M24SR      ");
	LCD_DisplayStringLine(Line6, "        ...         ");
}

/**
  * @brief  Screen config to inform user that AAR message has been stored in M24SR
  * @param  None
  * @retval None
  */
static void DisplayDwLdPhoneAppMessage( void)
{
	/* Set the Title Back Color */
  LCD_SetBackColor(Black);

  /* Set the Title Color */
  LCD_SetTextColor(White);
	
	/* Clear the LCD Screen */
  LCD_Clear(Blue);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Text Back Color */
  LCD_SetBackColor(Blue);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line2, "   AAR record has   ");
	LCD_DisplayStringLine(Line3, " been downloaded in ");
	LCD_DisplayStringLine(Line4, " M24SR. Please wave ");
	LCD_DisplayStringLine(Line5, "  your phone near   ");
	LCD_DisplayStringLine(Line6, "antenna to launch or");
	LCD_DisplayStringLine(Line7, " download M24SR App ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
	/* External Interrupt Enable */
	IntExtOnOffConfig(ENABLE);
	
	/* wait user action to go back to previous screen */
	WaitUserAction();
	
	/* External Interrupt Disable */
	IntExtOnOffConfig(DISABLE);		
	
	/* Clear the LCD Screen */
  LCD_Clear(White);
	/* Display Menu */
  DisplayMenu();
}

/**
  * @brief  Screen config to inform user that M24SR is still or no more locked
  * @param  ActionCompleted: true if unlocked, false if still locked
  * @retval None
  */
static void DisplayNormalMode( bool ActionCompleted)
{
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce();

	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	if( ActionCompleted == true)
	{
		LCD_DisplayStringLine(Line4, (u8*)"Limitation has been ");
		LCD_DisplayStringLine(Line5, (u8*)"      removed       ");
	}
	else
	{
		LCD_DisplayStringLine(Line4, (u8*)" Limitation has not ");
		LCD_DisplayStringLine(Line5, (u8*)"    been removed    ");
	}
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Screen config to inform user that M24SR is still or no more locked
  * @param  ActionCompleted: true if unlocked, false if still locked
  * @retval None
  */
static void DisplaylimitedAccess( bool ActionCompleted)
{
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	if( ActionCompleted == true)
	{
		LCD_DisplayStringLine(Line4, (u8*)"Limitation has been ");
		LCD_DisplayStringLine(Line5, (u8*)"      enabled       ");
	}
	else
	{
		LCD_DisplayStringLine(Line4, (u8*)" Limitation has not ");
		LCD_DisplayStringLine(Line5, (u8*)"     been enabled   ");
	}
		
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Screen config to inform user that M24SR is now locked with password
  * @param  NewPassWordSet: true if locked, false if still unlocked
	* @param  pPassword: pointer on the hex value of the password
  * @retval None
  */
static void DisplayLockPassword(bool NewPassWordSet, u8* pPassword)
{
	char Password[32];
		
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	if( NewPassWordSet)
	{
		Hex2Char( pPassword, 16, Password );
		
		LCD_DisplayStringLine(Line3, (u8*)"    New Password    ");
		LCD_DisplayStringLineWSize(Line4, (u8*)Password,16);	
		LCD_DisplayStringLineWSize(Line5, (u8*)(&Password[16]),16);	
		LCD_DisplayStringLine(Line6, (u8*)"     activated      ");
	}
	else
	{
		LCD_DisplayStringLine(Line3, (u8*)"    Action aborted  ");
		LCD_DisplayStringLine(Line4, (u8*)"  The password has  ");
		LCD_DisplayStringLine(Line5, (u8*)"been modified on RF ");
		LCD_DisplayStringLine(Line6, (u8*)"side, use SuperUser ");
		LCD_DisplayStringLine(Line7, (u8*)"mode to unlock M24SR");
	}
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
} 

/**
  * @brief  Screen config to inform user that M24SR is now unlocked
  * @param  PassWordState: true if unlocked, false if still locked
  * @retval None
  */
static void DisplayUnLockPassword(bool PassWordState)
{
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
	
	if( PassWordState)
	{
		LCD_DisplayStringLine(Line4, (u8*)"      Password      ");
		LCD_DisplayStringLine(Line5, (u8*)" has been disabled  ");
	}
	else
	{
		LCD_DisplayStringLine(Line3, (u8*)"    Action aborted  ");
		LCD_DisplayStringLine(Line4, (u8*)"  The password has  ");
		LCD_DisplayStringLine(Line5, (u8*)"been modified on RF ");
		LCD_DisplayStringLine(Line6, (u8*)"side, use SuperUser ");
		LCD_DisplayStringLine(Line7, (u8*)"mode to unlock M24SR");
	}
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}


/**
  * @brief  Displays the current menu.
  * @param  None
  * @retval None
  */
static void DisplayMenu(void)
{
  uint32_t Line = 0, index = 0;
  tMenuItem psMenuItem2;

  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
  /* Clear the LCD Screen */
  LCD_Clear(White);

  LCD_DisplayStringLine(Line, psCurrentMenu->pszTitle);
  Line += 2;

  /* Set the Back Color */
  LCD_SetBackColor(White);

  /* Set the Text Color */
  LCD_SetTextColor(Blue);

  while(!(index >= (psCurrentMenu->nItems)))
  {
    psMenuItem2 = &(psCurrentMenu->psItems[index]);
    LCD_DisplayStringLine(Line, psMenuItem2->pszTitle);
    index++;
    Line += 2;
  }
  /* Set the Back Color */
  LCD_SetBackColor(Green);

  /* Set the Text Color */
  LCD_SetTextColor(White);

  /* Get the current menu */
  psMenuItem = &(psCurrentMenu->psItems[MenuItemIndex]);

	LCD_DisplayStringLine((MenuItemIndex+1)*2 , psMenuItem->pszTitle);
}

/**
  * @brief  Idle function.
  * @param  None
  * @retval None
  */
static void IdleFunc(void)
{
  /* Nothing to execute: return */
  return;
}

/**
  * @brief  Check if URI is present in M24SR an display it.
  * @param  None
  * @retval None
  */
static void ReadURI( void)
{
	u16 status = ERROR;
	sURI_Info URI_struct;
	sURI_Info *pURI;
	
	pURI = &URI_struct;
	
	LCD_Clear(Blue);
	
	/* Set the Back Color */
	LCD_SetBackColor(Black);
	/* Set the Text Color */
	LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);

	WaitDebounce(); 
  
	LCD_DisplayStringLine(Line0, (u8*)"        URI         ");
	
	/* Set the Back Color */
	LCD_SetBackColor(Blue);
	
	pURI->protocol[0] = '\0';
	pURI->URI_Message[0] = '\0';
	memcpy(pURI->Information, "No Information\0",strlen("No Information\0")+1);
	
	status = TT4_ReadURI(pURI);
	
	if (status == SUCCESS)
	{
		DisplayURI(pURI);
	}
	else
	{
		LCD_Clear(Blue);
	
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		/* Set the Text Color */
		LCD_SetTextColor(White);
	
		IntExtOnOffConfig(DISABLE);

		WaitDebounce(); 
  
		LCD_DisplayStringLine(Line0, (u8*)"        URI         ");
	
		/* Set the Back Color */
		LCD_SetBackColor(Blue);
	
		LCD_DisplayStringLine(Line3, "   Data stored in   ");
		LCD_DisplayStringLine(Line4, "       M24SR        ");
		LCD_DisplayStringLine(Line5, " does not represent ");
		LCD_DisplayStringLine(Line6, "       an URI       ");
		
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
		/* wait user action to go back to previous screen */
		WaitUserAction();

		LCD_Clear(White);
		DisplayMenu();
		IntExtOnOffConfig(ENABLE);	
	}	
}

/**
  * @brief  Configured M24SR with URL.
  * @param  None
  * @retval None
  */
static void Load_URI_URL(void)
{
	u16 status = ERROR;
	sURI_Info URI_struct;
	sURI_Info *pURI;

	pURI = &URI_struct;
	
	memcpy(pURI->protocol, Protocol, strlen(Protocol)+1);
	memcpy(pURI->URI_Message, URI_Message, strlen(URI_Message)+1);
	status = TT4_WriteURI(pURI);
	
	
	if(status == SUCCESS)
		DisplayMessage("        URL         ");
	else
		DisplayErrorMessage();
}


/**
  * @brief  Configured M24SR with phone number.
  * @param  None
  * @retval None
  */
static void Load_URI_Tel(void)
{
	u16 status = ERROR;
	sURI_Info URI_struct;
	sURI_Info *pURI;
	
	pURI = &URI_struct;

	memcpy(pURI->protocol, "tel:\0", strlen("tel:\0")+1);
	memcpy(pURI->URI_Message, "0612345678\0", strlen("0612345678\0")+1);
	status = TT4_WriteURI(pURI);
	
	if(status == SUCCESS)
		DisplayMessage("    Phone number    ");
	else
		DisplayErrorMessage();

}

/**
  * @brief  Check if SMS is present in M24SR an display it.
  * @param  None
  * @retval None
  */
static void ReadSMS(void)
{
	u16 status = ERROR;
	sSMSInfo SMSStruct;
	sSMSInfo *pSMSStruct;

	pSMSStruct = &SMSStruct;
	
	pSMSStruct->PhoneNumber[0] = '\0';
	pSMSStruct->Message[0] = '\0';
	pSMSStruct->Information[0] = '\0';
	
	status = TT4_ReadSMS(pSMSStruct);
	
	if (status == SUCCESS)
		DisplaySMS(pSMSStruct);
	else
	{
		LCD_Clear(Blue);
	
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		/* Set the Text Color */
		LCD_SetTextColor(White);
	
		IntExtOnOffConfig(DISABLE);

		WaitDebounce(); 
  
		LCD_DisplayStringLine(Line0, (u8*)"        SMS         ");
	
		/* Set the Back Color */
		LCD_SetBackColor(Blue);
	
		LCD_DisplayStringLine(Line3, "   The NDEF file    ");
		LCD_DisplayStringLine(Line4, "  stored in M24SR   ");
		LCD_DisplayStringLine(Line5, " does not represent ");
		LCD_DisplayStringLine(Line6, "   a SMS message    ");
		
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
		/* wait user action to go back to previous screen */
		WaitUserAction();

		LCD_Clear(White);
		DisplayMenu();
		IntExtOnOffConfig(ENABLE);	
	}
}

/**
  * @brief  Configured M24SR with SMS.
  * @param  None
  * @retval None
  */
static void WriteSMS(void)
{
	u16 status = ERROR;
	sSMSInfo SMSStruct;
	sSMSInfo *pSMSStruct;
	
	pSMSStruct = &SMSStruct;
	
	memcpy(pSMSStruct->PhoneNumber, PhoneNumber, strlen(PhoneNumber)+1);
	memcpy(pSMSStruct->Message, Message, strlen(Message)+1);
	memcpy(pSMSStruct->Information, Instruction, strlen(Instruction)+1);	
	
	status = TT4_WriteSMS ( pSMSStruct );
	
	if(status == SUCCESS)
		DisplayMessage("        SMS         ");
	else
		DisplayErrorMessage();
	
}

/**
  * @brief  Check if Email is present in M24SR an display it.
  * @param  None
  * @retval None
  */
static void ReadEmail(void)
{	
	sEmailInfo EmailStruct;
	sEmailInfo *pEmailStruct;

	pEmailStruct = &EmailStruct;
	pEmailStruct->EmailAdd[0] = '\0';
	pEmailStruct->Subject[0] = '\0';
	pEmailStruct->Message[0] = '\0';
	pEmailStruct->Information[0] = '\0';
	
	if (	TT4_ReadEmail( pEmailStruct ) == SUCCESS)
	{
		DisplayEmail(pEmailStruct);
	}
	else
	{
		LCD_Clear(Blue);
	
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		/* Set the Text Color */
		LCD_SetTextColor(White);
	
		IntExtOnOffConfig(DISABLE);

		WaitDebounce(); 
  
		LCD_DisplayStringLine(Line0, (u8*)"        EMAIL        ");
	
		/* Set the Back Color */
		LCD_SetBackColor(Blue);
	
		LCD_DisplayStringLine(Line3, "   The NDEF file    ");
		LCD_DisplayStringLine(Line4, "  stored in M24SR   ");
		LCD_DisplayStringLine(Line5, " does not represent ");
		LCD_DisplayStringLine(Line6, "  an Email message  ");
		
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
		/* wait user action to go back to previous screen */
		WaitUserAction();

		LCD_Clear(White);
		DisplayMenu();
		IntExtOnOffConfig(ENABLE);	
	}
}

/**
  * @brief  Configured M24SR with Email.
  * @param  None
  * @retval None
  */
static void WriteEmail(void)
{
	u16 status = ERROR;
	sEmailInfo EmailStruct;
	sEmailInfo *pEmailStruct;
	
	pEmailStruct = &EmailStruct;
	
	memcpy(pEmailStruct->EmailAdd, EmailAdd, strlen(EmailAdd)+1);
	memcpy(pEmailStruct->Subject, Subject, strlen(Subject)+1);
	memcpy(pEmailStruct->Message, EmailMessage, strlen(EmailMessage)+1);
	memcpy(pEmailStruct->Information, Information, strlen(Information)+1);

	
	status = TT4_WriteEmail ( pEmailStruct );
	
	if(status == SUCCESS)
		DisplayMessage("       Email         ");
	else
		DisplayErrorMessage();
}

/**
  * @brief  Check if Vcard is present in M24SR an display it.
  * @param  None
  * @retval None
  */
static void ReadVcard(void)
{	
	sVcardInfo VcardStruct;
	sVcardInfo *pVcardStruct;
	
	pVcardStruct = &VcardStruct;
	
	pVcardStruct->FirstName[0] = '\0';
	pVcardStruct->Title[0] = '\0';
	pVcardStruct->Org[0] = '\0';
	pVcardStruct->HomeAddress[0] = '\0';
	pVcardStruct->WorkAddress[0] = '\0';
	pVcardStruct->HomeTel[0] = '\0';
	pVcardStruct->WorkTel[0] = '\0';
	pVcardStruct->CellTel[0] = '\0';
	pVcardStruct->HomeEmail[0] = '\0';
	pVcardStruct->WorkEmail[0] = '\0';

	if (TT4_ReadVcard( pVcardStruct ) != SUCCESS)
	{
		memcpy(pVcardStruct->FirstName, "NDEF is not a Vcard ", 20);
	}
	
	DisplayVcard(&VcardStruct);
}

/**
  * @brief  Configured M24SR with Vcard.
  * @param  None
  * @retval None
  */
static void WriteVcard(void)
{
	u16 status = ERROR;
	sVcardInfo VcardStruct;
	sVcardInfo *pVcardStruct;
	
	pVcardStruct = &VcardStruct;
	
	memcpy(pVcardStruct->FirstName, FirstName, strlen(FirstName)+1);
	memcpy(pVcardStruct->Title, Title, strlen(Title)+1);
	memcpy(pVcardStruct->Org, Org, strlen(Org)+1);
	memcpy(pVcardStruct->HomeAddress, HomeAddress, strlen(HomeAddress)+1);
	memcpy(pVcardStruct->WorkAddress, WorkAddress, strlen(WorkAddress)+1);
	memcpy(pVcardStruct->HomeTel, HomeTel, strlen(HomeTel)+1);
	memcpy(pVcardStruct->WorkTel, WorkTel, strlen(WorkTel)+1);
	memcpy(pVcardStruct->CellTel, CellTel, strlen(CellTel)+1);
	memcpy(pVcardStruct->HomeEmail, HomeEmail, strlen(HomeEmail)+1);
	memcpy(pVcardStruct->WorkEmail, WorkEmail, strlen(WorkEmail)+1);
	
	DisplayMessageOnGoing();
	
	//status = TT4_WriteVcard ( &VcardStruct );
	/* for demonstration force a NDEF file Vcard with a default Jpeg embedded */
	status = TT4_WriteNDEF((u8*)uVcardCSL1);
	
	if(status == SUCCESS)
		DisplayMessage("       Vcard        ");
	else
		DisplayErrorMessage();
}
	
/**
  * @brief  Check if geolocation is present in M24SR an display it.
  * @param  None
  * @retval None
  */
static void ReadGeo(void)
{
	u16 status = ERROR;
	sGeoInfo GeoStruct;

	status = TT4_ReadGeo(&GeoStruct);
	
	if (status == SUCCESS)
		DisplayGeo(&GeoStruct);
	else
	{
		LCD_Clear(Blue);
	
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		/* Set the Text Color */
		LCD_SetTextColor(White);
	
		IntExtOnOffConfig(DISABLE);

		WaitDebounce(); 
  
		LCD_DisplayStringLine(Line0, "    Geolocation     ");
	
		/* Set the Back Color */
		LCD_SetBackColor(Blue);
	
		LCD_DisplayStringLine(Line3, "   The NDEF file    ");
		LCD_DisplayStringLine(Line4, "  stored in M24SR   ");
		LCD_DisplayStringLine(Line5, " does not represent ");
		LCD_DisplayStringLine(Line6, "   a GPS position   ");
		
		/* Set the Back Color */
		LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
		/* wait user action to go back to previous screen */
		WaitUserAction();

		LCD_Clear(White);
		DisplayMenu();
		IntExtOnOffConfig(ENABLE);	
	}
}

/**
  * @brief  Configured M24SR with Geolocation.
  * @param  None
  * @retval None
  */
static void WriteGeo(void)
{
	u16 status = ERROR;
	sGeoInfo GeoStruct;
	sGeoInfo *pGeoStruct;
	pGeoStruct = &GeoStruct;
	
	memcpy(pGeoStruct->Latitude, Latitude, strlen(Latitude)+1);
	memcpy(pGeoStruct->Longitude, Longitude, strlen(Longitude)+1);
	memcpy(pGeoStruct->Information, GeoInformation, strlen(GeoInformation)+1);	
	
	status = TT4_WriteGeo ( pGeoStruct );
	
	if(status == SUCCESS)
		DisplayMessage("    Geolocation     ");
	else
		DisplayErrorMessage();
	
}

/**
  * @brief  Supress M24SR content and replace by AAR alone.
  * @param  None
  * @retval None
  */
static void DWLoadPhoneApp(void)
{
	u8 NULL_NDEF[2] = {0,0};
	u16 status = ERROR;
	sAARInfo AAR_struct;
	
	sAARInfo *pAAR;
	
	pAAR = &AAR_struct;
	
	/* to set only AAR force NDEF size to 0 before */
	TT4_WriteNDEF(NULL_NDEF);
	
	memcpy(pAAR->PakageName, "com.nfc.m24srdemo", strlen("com.nfc.m24srdemo")+1);
	
	/* Add AAR record */
	status = TT4_AddAAR(pAAR);
	
	if(status == SUCCESS)
		DisplayDwLdPhoneAppMessage();
	else
		DisplayErrorMessage();
	
}


/**
  * @brief  Prepare M24SR-DISCOVERY to interact with private application.
  * @param  None
  * @retval None
  */
static void LaunchDemoApp()
{
	u16 status = ERROR;
	u8 NULL_NDEF[2] = {0,0};
	Led_TypeDef i;
	u8 j = 0;
	u16 counter, BlinkCounter;
	u8 event = NO_EVENT;
	sAARInfo AAR_struct;
	sAARInfo *pAAR;
	sMyAppInfo MyApp_struct;
	sMyAppInfo *pMyAppStruct;
	
	pMyAppStruct = &MyApp_struct;
	
	
	/* Save Led context */
	for(i=LED1; i<LED5; i++)
	{
		if(IsLED_On(i))
			LedContext[i] = true;
		else
			LedContext[i] = false;
	}
	
	
	/* As this demonstration need M24SR application force M24SR demo launch with AAR */
	pAAR = &AAR_struct;
	/* to set only AAR force NDEF size to 0 before */
	TT4_WriteNDEF(NULL_NDEF);
	memcpy(pAAR->PakageName, "com.nfc.m24srdemo", strlen("com.nfc.m24srdemo")+1);
	/* Add AAR record */
	TT4_AddAAR(pAAR);
	
	/* configure M24SR to have GPO going low when RF update M24SR */
	STProprietary_GPOConfig(WIP, RF_GPO);
	
	LCD_Clear(Blue);
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	IntExtOnOffConfig(DISABLE);
	
	LCD_DisplayStringLine(Line0, (u8*)"  M24SR-DISCOVERY   ");
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	

  WaitDebounce(); 
	 	
	
	/* During I2C access, RF access can not be perform */
	/* With M24SR full it's difficult for RF reader to take hand on M24SR with this implementation */
	/* In the next loop the time to read will be longer than the time let for RF access (100ms) */
	/* So set NDEF size to 0, to simulate a M24SR empty */
	//TT4_WriteNDEF(NULL_NDEF);
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	LCD_DisplayStringLine(Line3, (u8*)"    Waiting for     ");
	LCD_DisplayStringLine(Line4, (u8*)"   application to   ");
	LCD_DisplayStringLine(Line5, (u8*)"update M24SR content");
	
	/* retrieve NDEF information */	
	status = TT4_ReadMyApp ( pMyAppStruct );
	
	event = NO_EVENT;	
	while( event != KEY_EVENT)
	{
		if (event == RF_EVENT)
		{
			/* few ms to let write operation complete */
			delay_ms(1000);
			/* retrieve NDEF information */	
			status = TT4_ReadMyApp ( pMyAppStruct );
				
			event = NO_EVENT;
		}
		
		if( status != SUCCESS)
		{
			event=NO_EVENT;
			while(event == NO_EVENT)
			{
				counter = 1000; //100ms
				while( counter )
				{
					delay_us(100);
					counter--;
					event = Event();
					if( event != NO_EVENT)
						break;
				}				
			}
		}
		else
		{
			for(j=0; j<8; j++)
			{
				/* Set the Back Color */
				LCD_SetBackColor(pMyAppStruct->LineX[j].BackGroundColor);
				/* Set the Text Color */
				LCD_SetTextColor(pMyAppStruct->LineX[j].FontColor);
				/* Display the string */
				LCD_DisplayStringLine(pMyAppStruct->LineX[j].LineNb, (u8*)(pMyAppStruct->LineX[j].String));
			}		
		
			switch(pMyAppStruct->LedBlinkConf.Speed)
			{
				case BLINKING_NONE:
					BlinkCounter = 0;
					break;
			
				case BLINKING_SLOW:
					BlinkCounter = 5000;
					break;
			
				case BLINKING_MEDIUM:
					BlinkCounter = 2500;
					break;
			
				case BLINKING_FAST:
					BlinkCounter = 500;
					break;
			}
		
			//initialization
			for(i=LED1; i<LED5; i++)
			{
				if( ((pMyAppStruct->LedBlinkConf.LedConf[i])&(0x01)) == 0x00)
					LED_Off(i);
				if( ((pMyAppStruct->LedBlinkConf.LedConf[i])&(0x01)) == 0x01)
					LED_On(i);
			}

			/* wait user action to go back to previous screen */
			event=NO_EVENT;
			while(event == NO_EVENT)
			{			
				for(i=LED1; i<LED5; i++)
				{
					if( ((pMyAppStruct->LedBlinkConf.LedConf[i])&(0x02))  == 0x02 && BlinkCounter )
					{
						if( IsLED_On(i) == true)
							LED_Off(i);			
						else
							LED_On(i);
				
						/* delay Led ON */
						counter = BlinkCounter;
						while( counter )
						{
							delay_us(100);
							counter--;
							event = Event();
							if( event != NO_EVENT)
								break;
						}							
					}
					else
					{
						event = Event();
					}
				
					if( event != NO_EVENT)
						break;
				}		
			}		
		}

}
	
	LCD_Clear(White);
	DisplayMenu();
	
	/* restore default RF GPO config */
	STProprietary_GPOConfig(SESSION_OPENED, RF_GPO);
	
	/* Restore Led context */
	for(i=LED1; i<LED5; i++)
	{
		if(LedContext[i])
			LED_On(i);
		else
			LED_Off(i);
	}
	IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Activate M24SR Read Password
  * @param  None
  * @retval None
  */
static void EnableReadPWD (void)
{
	u16 status, i;
	u32 randomKey;
	u8 NewReadPassword[16];
	
	randomKey = GetElapsed_us();
	
  for(i=0; i<16; i+=4)
	{
		NewReadPassword[i]= (u8)(randomKey & 0x000000FF);
		NewReadPassword[i+1]= (u8)((randomKey & 0x0000FF00)>>8);
		NewReadPassword[i+2]= (u8)((randomKey & 0x00FF0000)>>16);
		NewReadPassword[i+3]= (u8)((randomKey & 0xFF000000)>>24);
		randomKey += (randomKey << 8);
	}
	
	status = ISO7816_EnableReadPwd(CurrentWritePassword, NewReadPassword);
	
	if(status == SUCCESS)
	{	
		ReadPasswordActivated = true;
		LED_On(LED1);
		
		for(i=0; i<16; i++)
		{
			CurrentReadPassword[i] = NewReadPassword[i];
		}
	
		DisplayLockPassword( true, NewReadPassword );
	}
	else
	{
		DisplayLockPassword( false, NewReadPassword);
	}
	
}

/**
  * @brief  Desactivate M24SR Read Password
  * @param  None
  * @retval None
  */
static void DisableReadPWD (void)
{
	u16 status;

	status = ISO7816_DisableReadPwd(CurrentWritePassword);
		
	if(status == SUCCESS)
	{
		ReadPasswordActivated = false;
		if(WritePasswordActivated == false && ReadOnlyModeActivated == false && WriteOnlyModeActivated == false)
			LED_Off(LED1);
	
		DisplayUnLockPassword( true);
	}
	else
		DisplayUnLockPassword( false);
}

/**
  * @brief  Activate M24SR Write Password
  * @param  None
  * @retval None
  */
static void EnableWritePWD (void)
{
	u16 status, i;
	u32 randomKey;
	u8 NewPassword[16];
	
	randomKey = GetElapsed_us();
	
  for(i=0; i<16; i+=4)
	{
		NewPassword[i] = (u8)(randomKey & 0x000000FF);
		NewPassword[i+1] = (u8)((randomKey & 0x0000FF00)>>8);
		NewPassword[i+2] = (u8)((randomKey & 0x00FF0000)>>16);
		NewPassword[i+3] = (u8)((randomKey & 0xFF000000)>>24);
		randomKey += (randomKey << 8);
	}
	
	status = ISO7816_EnableWritePwd(CurrentWritePassword, NewPassword);
	
	if(status == SUCCESS)
	{	
		WritePasswordActivated = true;
		LED_On(LED1);
		
		for(i=0; i<16; i++)
		{
			CurrentWritePassword[i] = NewPassword[i];
		}
	
		DisplayLockPassword( true, NewPassword );
	}
	else
	{
		DisplayLockPassword( false, NewPassword);
	}
}

/**
  * @brief  Desactivate M24SR Write Password
  * @param  None
  * @retval None
  */
static void DisableWritePWD (void)
{
	u16 status;

	status = ISO7816_DisableWritePwd(CurrentWritePassword);
		
	if(status == SUCCESS)
	{
		WritePasswordActivated = false;
		if(ReadPasswordActivated == false && ReadOnlyModeActivated == false && WriteOnlyModeActivated == false)
			LED_Off(LED1);		
		DisplayUnLockPassword( true);
	}
	else
		DisplayUnLockPassword( false);
}

/**
  * @brief  Disable all M24SR lock by using I2C superuser password
  * @param  None
  * @retval None
  */
static void SuperUser (void)
{
	uint8_t ResetI2CPassword[16]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
														0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	u16 status, i;
	
	status = ISO7816_DisableAllPassword(ResetI2CPassword);
	if(status == SUCCESS)
	{
		ReadPasswordActivated = false;
		WritePasswordActivated = false;
		ReadOnlyModeActivated = false;
		WriteOnlyModeActivated = false;
		LED_Off(LED1);		
		for(i=0; i<16; i++)
		{
			CurrentReadPassword[i]=0;
			CurrentWritePassword[i]=0;
		}
		
		DisplayUnLockPassword( true);
	}
	//else
	//	DisplayUnLockPassword( false);
}

/**
  * @brief  Set M24SR in Read Only Mode
  * @param  None
  * @retval None
  */
static void EnableReadOnlyMode (void)
{
	u16 status;
	
	status = STProprietary_EnableReadOnly(CurrentWritePassword);
	if(status == SUCCESS)
	{
		ReadOnlyModeActivated = true;
			LED_On(LED1);		
		DisplaylimitedAccess( true);
	}
	else
		DisplaylimitedAccess( false);
	
}

/**
  * @brief  Disable Read Only Mode
  * @param  None
  * @retval None
  */
static void DisableReadOnlyMode (void)
{
	u16 status;
	
	status = STProprietary_DisableReadOnly(CurrentWritePassword);
	if(status == SUCCESS)
	{
		ReadOnlyModeActivated = false;
		if(ReadPasswordActivated == false && WritePasswordActivated == false && WriteOnlyModeActivated == false)
			LED_Off(LED1);		
		DisplayNormalMode( true);
	}
	else
		DisplayNormalMode( false);
	
}

/**
  * @brief  Set M24SR in Write Only Mode
  * @param  None
  * @retval None
  */
static void EnableWriteOnlyMode (void)
{
	u16 status;
	
	status = STProprietary_EnableWriteOnly(CurrentWritePassword);
	if(status == SUCCESS)
	{
		WriteOnlyModeActivated = true;
			LED_On(LED1);		
		DisplaylimitedAccess( true);
	}
	else
		DisplaylimitedAccess( false);
	
}

/**
  * @brief  disable Write Only Mode
  * @param  None
  * @retval None
  */
static void DisableWriteOnlyMode (void)
{
	u16 status;
	
	status = STProprietary_DisableWriteOnly(CurrentWritePassword);
	if(status == SUCCESS)
	{
		WriteOnlyModeActivated = false;
		if(ReadPasswordActivated == false && WritePasswordActivated == false && ReadOnlyModeActivated == false)
			LED_Off(LED1);		
		DisplayNormalMode( true);
	}
	else
		DisplayNormalMode( false);
	
}

/**
  * @brief  Display information of the song played on smartphone
  * @param  None
  * @retval None
  */	
static void Display_Info(void)
{
	char DisplayBuffer[100];
	uint32_t TitleOffset =0, ArtistOffset=0, AlbumOffset=0, GenreOffset=0;
	LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce(); 
	
  /* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	LCD_DisplayStringLine(Line0, (u8*)"  Song Information  ");
	
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
	
	/* Set the Back Color */
  LCD_SetBackColor(Blue);
		
	while(ReadKey() == NOKEY)
  {
			
		if(strlen((char*)&(MediaFile.Title)))
		{
			memcpy((char*)(DisplayBuffer),"Title: ",7);
			memcpy((char*)&(DisplayBuffer[7]),(char*)&(MediaFile.Title),strlen((char*)&(MediaFile.Title)));
			
			if(strlen((char*)&(MediaFile.Title)) > (20-7) && TitleOffset < strlen((char*)&(MediaFile.Title))+7-15)
			{
				LCD_DisplayStringLineWSize(Line2, (u8*)(&DisplayBuffer[TitleOffset]),strlen((char*)&(MediaFile.Title))+7-TitleOffset);
				TitleOffset++;
			}
			else
			{
				TitleOffset=0;
				LCD_DisplayStringLineWSize(Line2, (u8*)(DisplayBuffer),strlen((char*)&(MediaFile.Title))+7);
			}
		}
		else
			LCD_DisplayStringLine(Line2, "Title:              ");
	
		if(strlen((char*)&(MediaFile.Artist)))
		{
			memcpy((char*)(DisplayBuffer),"Artist: ",8);
			memcpy((char*)&(DisplayBuffer[8]),(char*)&(MediaFile.Artist),strlen((char*)&(MediaFile.Artist)));
			if(strlen((char*)&(MediaFile.Artist)) > (20-8) && ArtistOffset < strlen((char*)&(MediaFile.Artist))+8-15)
			{
				LCD_DisplayStringLineWSize(Line3, (u8*)(&DisplayBuffer[ArtistOffset]),strlen((char*)&(MediaFile.Artist))+8-ArtistOffset);
				ArtistOffset++;
			}
			else
			{
				ArtistOffset=0;
				LCD_DisplayStringLineWSize(Line3, (u8*)(DisplayBuffer),strlen((char*)&(MediaFile.Artist))+8);
			}		
		}
		else
			LCD_DisplayStringLine(Line3, "Artist:             ");
	
		if(strlen((char*)&(MediaFile.Album)))
		{
			memcpy((char*)(DisplayBuffer),"Album: ",7);
			memcpy((char*)&(DisplayBuffer[7]),(char*)&(MediaFile.Album),strlen((char*)&(MediaFile.Album)));
			if(strlen((char*)&(MediaFile.Album)) > (20-7) && AlbumOffset < strlen((char*)&(MediaFile.Album))+7-15)
			{
				LCD_DisplayStringLineWSize(Line4, (u8*)(&DisplayBuffer[AlbumOffset]),strlen((char*)&(MediaFile.Album))+7-AlbumOffset);
				AlbumOffset++;
			}
			else
			{
				AlbumOffset=0;
				LCD_DisplayStringLineWSize(Line4, (u8*)(DisplayBuffer),strlen((char*)&(MediaFile.Album))+7);
			}		
		}
		else
			LCD_DisplayStringLine(Line4, "Album:              ");
	
		if(strlen((char*)&(MediaFile.Number)))
		{
			memcpy((char*)(DisplayBuffer),"Piste: ",7);
			memcpy((char*)&(DisplayBuffer[7]),(char*)&(MediaFile.Number),strlen((char*)&(MediaFile.Number)));
			memcpy((char*)&(DisplayBuffer[7+strlen((char*)&(MediaFile.Number))]),"/",1);
			memcpy((char*)&(DisplayBuffer[7+strlen((char*)&(MediaFile.Number))+1]),(char*)&(MediaFile.Total_Number),strlen((char*)&(MediaFile.Total_Number)));
			LCD_DisplayStringLineWSize(Line5, (u8*)(DisplayBuffer),strlen((char*)&(MediaFile.Total_Number))+1+strlen((char*)&(MediaFile.Number))+7);
		}
		else
			LCD_DisplayStringLine(Line5, "Piste:              ");
	
		if(strlen((char*)&(MediaFile.Genre)))
		{
			memcpy((char*)(DisplayBuffer),"Genre: ",7);
			memcpy((char*)&(DisplayBuffer[7]),(char*)&(MediaFile.Genre),strlen((char*)&(MediaFile.Genre)));
			if(strlen((char*)&(MediaFile.Genre)) > (20-7) && GenreOffset < strlen((char*)&(MediaFile.Genre))+7-15)
			{
				LCD_DisplayStringLineWSize(Line6, (u8*)(&DisplayBuffer[GenreOffset]),strlen((char*)&(MediaFile.Genre))+7-GenreOffset);
				GenreOffset++;
			}
			else
			{
				GenreOffset=0;
				LCD_DisplayStringLineWSize(Line6, (u8*)(DisplayBuffer),strlen((char*)&(MediaFile.Genre))+7);
			}			
		}
		else
			LCD_DisplayStringLine(Line6, "Genre:              ");
	
		if(strlen((char*)&(MediaFile.PlayingTime)))
		{
			memcpy((char*)(DisplayBuffer),"Time: ",6);
			memcpy((char*)&(DisplayBuffer[6]),(char*)&(MediaFile.PlayingTime),strlen((char*)&(MediaFile.PlayingTime)));
			LCD_DisplayStringLineWSize(Line7, (u8*)(DisplayBuffer),strlen((char*)&(MediaFile.PlayingTime))+6);
		}
		else
			LCD_DisplayStringLine(Line7, (u8*)"Time:               ");
	
  }

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Configured M24SR with Mac Address of the BT module.
  * @param  None
  * @retval None
  */
static void Load_BT_NDEF(void)
{
	char MacAddr[30];
	uint32_t MacAddSize, i;
	
	/* Enable BT_pairing */
	BT_Enable();
	
	BT_GetMacAddr((char*)(MacAddr));
	MacAddSize = strlen((char*)(MacAddr));
	
	/* convert char Mac Address in hex */
	Char2Hex(MacAddr, MacAddSize);
	
	/* format NDEF message for BT pairing */
	for(i=0;i<6;i++)
	{
		NDEF_BT_PAIRING[39+i] = MacAddr[i];
	}
	
	TT4_WriteNDEF(NDEF_BT_PAIRING);
	
	LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce();
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);

  LCD_DisplayStringLine(Line2, "  M24SR configured  ");
  LCD_DisplayStringLine(Line3, "   with Bluetooth   ");
  LCD_DisplayStringLine(Line4, "     parameters     ");
	
	LCD_DisplayStringLine(Line5, "  Please get close  ");
  LCD_DisplayStringLine(Line6, "  to antenna for    ");
  LCD_DisplayStringLine(Line7, "      pairing       ");
 
  /* Set the Back Color */
	LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
  /* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Allow new device to pair with BT module
  * @param  None
  * @retval None
  */
static void Enable_BT_Pairing(void)
{
	BT_Enable();
	
	LED_On(LED3);
	
	LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce();
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);

  LCD_DisplayStringLine(Line4, "   New device can   ");
	LCD_DisplayStringLine(Line5, "     be paired      ");
  
	

  /* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  M24SR can be access by RF
  * @param  None
  * @retval None
  */
static void Enable_M24SR_RF(void)
{
	M24SR_RFConfig(1);
	
	LED_On(LED2);
	
	LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce();
	
	/* Set the Back Color */
	LCD_SetBackColor(Black);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	LCD_DisplayStringLine(Line3, "      M24SR RF      ");
  LCD_DisplayStringLine(Line4, "  has been enabled  ");
  LCD_DisplayStringLine(Line5, "    get close to    ");
	LCD_DisplayStringLine(Line6, " antenna for action ");
  
	/* Set the Back Color */
	LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
	
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  M24SR can not be access by RF
  * @param  None
  * @retval None
  */
static void Disable_M24SR_RF(void)
{
	M24SR_RFConfig(0);
	
	LED_Off(LED2);
	
	LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce();
	
	/* Set the Back Color */
	LCD_SetBackColor(Black);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	 
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	LCD_DisplayStringLine(Line3, "      M24SR RF      ");
  LCD_DisplayStringLine(Line4, "  has been disabled ");
  LCD_DisplayStringLine(Line5, "    get close to    ");
	LCD_DisplayStringLine(Line6, " antenna is no use  ");
  
	/* Set the Back Color */
	LCD_SetBackColor(Black);
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	
	
	/* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Prepare M24SR-DISCOVERY to illustrate M24SR GPO controlled by RF
  * @param  None
  * @retval None
  */
static void Enable_GPO_RF(void)
{
	u8 NULL_NDEF[2] = {0,0};
	sAARInfo AAR_struct;
	u32 cmpt =0;
	u8 event = NO_EVENT;

	/* As this demonstration need M24SR application force M24SR demo launch with AAR */
	sAARInfo *pAAR;
	pAAR = &AAR_struct;
	/* to set only AAR force NDEF size to 0 before */
	TT4_WriteNDEF(NULL_NDEF);
	memcpy(pAAR->PakageName, "com.nfc.m24srdemo", strlen("com.nfc.m24srdemo")+1);
	/* Add AAR record */
	TT4_AddAAR(pAAR);
	
	/* Configure M24SR to allow GPO control by RF */
	STProprietary_GPOConfig(STATE_CONTROL, RF_GPO);
	
	LCD_Clear(Blue);
	
	IntExtOnOffConfig(DISABLE);

  WaitDebounce();
	
	/* Set the Back Color */
  LCD_SetBackColor(Black);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	LCD_DisplayStringLine(Line9, (u8*)"Press key to return ");
	 
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);
	
	LCD_DisplayStringLine(Line2, "    Waiting for     ");
	LCD_DisplayStringLine(Line3, "   application to   ");
	LCD_DisplayStringLine(Line4, " generate interrupt ");
  LCD_DisplayStringLine(Line5, "with M24SR GPO line ");
 
	LCD_SetTextColor(Red);
	LCD_DrawFullRect(12, 145, 64, 65);		
	LCD_DrawFullRect(88, 145, 64, 65);			
	LCD_DrawFullRect(164, 145, 64, 65);			
	LCD_DrawFullRect(240, 145, 64, 65);			
	
	/* wait user action to go back to previous screen */
	while( event != KEY_EVENT)
	{
	
		while( GPIO_ReadInputDataBit(M24SR_GPO_PIN_PORT,M24SR_GPO_PIN) != Bit_RESET )
		{
			event = Event();
			if( event == KEY_EVENT)
					break;
		};
		if( event == KEY_EVENT)
					break;
		
		if(cmpt%8 == 0)
		{
			LCD_SetTextColor(Green);
			LCD_DrawFullRect(12, 145, 64, 65);	
		}
		else if (cmpt%8 == 1)
		{
			LCD_SetTextColor(Green);
			LCD_DrawFullRect(88, 145, 64, 65);	
		}
		else if (cmpt%8 == 2)
		{
			LCD_SetTextColor(Green);
			LCD_DrawFullRect(164, 145, 64, 65);	
		}
		else if (cmpt%8 == 3)
		{
			LCD_SetTextColor(Green);
			LCD_DrawFullRect(240, 145, 64, 65);	
		}
		else if (cmpt%8 == 4)
		{
			LCD_SetTextColor(Red);
			LCD_DrawFullRect(12, 145, 64, 65);	
		}
		else if (cmpt%8 == 5)
		{
			LCD_SetTextColor(Red);
			LCD_DrawFullRect(88, 145, 64, 65);	
		}
		else if (cmpt%8 == 6)
		{
			LCD_SetTextColor(Red);
			LCD_DrawFullRect(164, 145, 64, 65);	
		}
		else if (cmpt%8 == 7)
		{
			LCD_SetTextColor(Red);
			LCD_DrawFullRect(240, 145, 64, 65);	
		}
		
		cmpt++;
		
		while( GPIO_ReadInputDataBit(M24SR_GPO_PIN_PORT,M24SR_GPO_PIN) != Bit_SET )
		{
			event = Event();
			if( event == KEY_EVENT)
					break;
		};
  };

	STProprietary_GPOConfig(SESSION_OPENED, RF_GPO);
	
  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Display the About menu.
  * @param  None
  * @retval None
  */
static void AboutFunc(void)
{
  LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce();
  
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);

  LCD_ClearLine(Line0);
  LCD_DisplayStringLine(Line1, "       M24SR        ");
  LCD_DisplayStringLine(Line2, "   Demonstration    ");
  LCD_DisplayStringLine(Line3, "   Version 2.0.0    ");
  LCD_ClearLine(Line4);
  LCD_DisplayStringLine(Line5, "      MMY Team      ");
  LCD_DisplayStringLine(Line6, "   COPYRIGHT 2016   ");
  LCD_DisplayStringLine(Line7, " STMicroelectronics ");
  LCD_ClearLine(Line8);
  LCD_DisplayStringLine(Line9, " www.st.com/nfc-rfid");

  /* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Runs the Product Presentation menu.
  * @param  None
  * @retval None
  */
static void ProductPres(void)
{
  __IO uint32_t index = 0, KeyState = 0, err = 0;
	
	LCD_Clear(Blue);

  IntExtOnOffConfig(DISABLE);

  WaitDebounce();

	/* Set the Back Color */
  LCD_SetBackColor(Black);
	/* Set the Text Color */
  LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0, "  M24SR-DISCOVERY   ");
	LCD_DisplayStringLine(Line9, " Press SEL to start ");  

  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);


  LCD_DisplayStringLine(Line2, " When presentation  ");
  LCD_DisplayStringLine(Line3, "  starts use RIGHT  "); 
  LCD_DisplayStringLine(Line4, " and LEFT to go to  ");
  LCD_DisplayStringLine(Line5, " the next/previous  ");
  LCD_DisplayStringLine(Line6, "  slide and SEL to  ");
  LCD_DisplayStringLine(Line7, "        exit.       ");

  while(ReadKey() != SEL)
  {
  }
  while(ReadKey() != NOKEY)
  {
  }
  
  while(index < M24SR_SLIDE_PRESENTATION)
  {  
  
    LCD_SetDisplayWindow(0, 0, 240, 320);
    /* Slide nÝ: index */  
		LCD_WriteBMP_FAT(0,0, (const char *) (SlideAddr[index]));
		LCD_WriteBMP_FAT(160,0, (const char *) (SlideAddr[index+1]));
    
    /* Clear the SysTick Counter */
    SysTick->VAL = (uint32_t)0x0;
    
    KeyState = DelayJoyStick(DelayTiming[index]);

    if(KeyState == LEFT)
    {
      if(index == 0)
      {
        index = 2;
      }
      index-=2;
    }
    else if (KeyState == RIGHT)
    {
      if(index == 12)
      { 
        index = 15;
      }
      index+=2;
    }
    else if (KeyState == SEL)
    {
      index = 15;
    }
    else if (KeyState == NOKEY)
    {

    }
  }

  /* Disable the SysTick Counter */
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE;

  /* Clear the SysTick Counter */
  SysTick->VAL = (uint32_t)0x0;

  LCD_Clear(Black);

  /* Set the Back Color */
  LCD_SetBackColor(Black);

  /* Set the Text Color */
  LCD_SetTextColor(White);

  LCD_DisplayStringLine(Line0, "  End of slide show ");
  LCD_DisplayStringLine(Line1, "Exit:  Push JoyStick");

  /* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}


/**
  * @brief  Runs the Antenna Presentation.
  * @param  None
  * @retval None
  */
static void AntennaPres(void)
{
	LCD_Clear(Blue);
	
  IntExtOnOffConfig(DISABLE);

  WaitDebounce();

  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White);


	LCD_DisplayStringLine(Line3, "    Your antenna    "); 
	LCD_DisplayStringLine(Line4, "  design made easy  "); 
  LCD_DisplayStringLine(Line5, "       with         ");
  LCD_DisplayStringLine(Line6, "    MATRIX-M24SR    "); 


	delay_ms(2000);
  
  LCD_SetDisplayWindow(0, 0, 240, 320);
 
	LCD_WriteBMP_FAT(0,0, (const char *) (uAntenna1));
	LCD_WriteBMP_FAT(160,0, (const char *) (uAntenna2));

  /* wait user action to go back to previous screen */
	WaitUserAction();

  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/**
  * @brief  Initializes the navigation menu.
  * @param  None
  * @retval None
  */
static void Menu_Init(void)
{
  psCurrentMenu = &MainMenu;
  psPrevMenu[nMenuLevel] = psCurrentMenu;
  psMenuItem = MainMenuItems;	
}

/**
  * @brief  Display menu icons.
  * @param  None
  * @retval None
  */
static void DisplayIcons(void)
{
  uint32_t i = 0;
	
	for(i=0; i<NumberOfIcons; i++)
	{
		/* if BT but no BT available on premium edition */
		if( i == 4 && !IsBTModulePresent())
		{
		  /* nothing to display as BT module not available */
		}
		else
		{
			if( IconsAddr[i] [0] != NULL)
			{	
				LCD_WriteBMP_FAT(IconRect[i][0]+2, IconRect[i][1]+3, (const char*)(IconsAddr[i][0]));
			}
		}
	}	
}

/**
  * @brief  This function is executed when the "RETURN" menu is selected.
  * @param  None
  * @retval None
  */
static void ReturnFunc(void)
{
  psMenuItem->pfUpDownMenuFunc();

  if(nMenuLevel == 0)
  {
    nMenuLevel++;
  }

  psCurrentMenu = psPrevMenu[nMenuLevel-1];
  psMenuItem = &psCurrentMenu->psItems[0];
  ItemNumb[nMenuLevel] = 0;
  MenuItemIndex = 0;
  nMenuLevel--;

  if(nMenuLevel != 0)
  {
    DisplayMenu();
  }
  else
  {
    ShowMenuIcons();
  }
}

/**
  * @brief  Inserts a delay time while no joystick RIGHT, LEFT and SEL 
  * @param  nTime: specifies the delay time length (time base 10 ms).
  * @retval None
	*/
static uint32_t DelayJoyStick(__IO uint32_t nTime)
{
  __IO uint32_t keystate = 0;

  /* Enable the SysTick Counter */
  SysTick->CTRL |= SysTick_CTRL_ENABLE;

  TimingDelay = nTime;
 
  while((TimingDelay != 0) && (keystate != RIGHT) && (keystate != LEFT) && (keystate != SEL))
  {
    keystate = ReadKey();
  } 
  
  return keystate;
}

/**
  * @}
  */


/** @addtogroup Menu_Public_Functions
 * 	@{
 */

/**
  * @brief  Display the ST introduction.
  * @param  None
  * @retval None
  */
void STIntro(void)
{	
#ifndef DEBUG
		LCD_WriteBMP_FAT(40, 24, (const char*)(uSTlogo));
#endif
}

/**
  * @brief  Show the main menu icon.
  * @param  None
  * @retval None
  */
void ShowMenuIcons(void)
{
  uint32_t MyKey = 0; 
	uint8_t CurrentIcone = 0;	

  /* Disable the JoyStick interrupts */
  IntExtOnOffConfig(DISABLE);

  WaitDebounce();

  /* Initializes the Menu state machine */
  Menu_Init();

  MenuItemIndex = 0;
  
  /* Clear*/
  LCD_Clear(White);

  /* Set the Back Color */
  LCD_SetBackColor(Blue);

  /* Set the Text Color */
  LCD_SetTextColor(White);

  LCD_DisplayStringLine(Line0, psMenuItem->pszTitle);
  
  /* Set the Back Color */
  LCD_SetBackColor(White);

  /* Set the Text Color */
  LCD_SetTextColor(Blue);

  /* Displays Icons */    
  DisplayIcons();

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  /* Disable LCD Window mode */
  LCD_WindowModeDisable(); 


  LCD_DrawRect(IconRect[CurrentIcone][0], IconRect[CurrentIcone][1], 64, 65);
	

  while(GPIO_ReadInputDataBit(J_SEL_PORT, J_SEL_PIN) == RESET)
  {
  }
  /* Endless loop */
  while(1)
  {	
    /* Check which key is pressed */
    MyKey = ReadKey();

    /* If "UP" pushbutton is pressed */
    if(MyKey == UP)
    {
      /* Set the Text Color */
      LCD_SetTextColor(White);

      LCD_DrawRect(IconRect[CurrentIcone][0], IconRect[CurrentIcone][1], 64, 65);
			
			if(CurrentIcone<NbIconeByLine)
			{
				CurrentIcone += NumberOfIcons;
			}
					
			do
			{
				CurrentIcone = (CurrentIcone-NbIconeByLine)%NumberOfIcons;
			}while( IconsAddr[CurrentIcone] [0] == NULL);
			
			/* if BT but no BT available on premium edition */
			if( CurrentIcone == 4 && !IsBTModulePresent())
			{
				do
				{
					CurrentIcone = (CurrentIcone-NbIconeByLine)%NumberOfIcons;
				}while( IconsAddr[CurrentIcone] [0] == NULL);
			}			
			
    }
    /* If "DOWN" pushbutton is pressed */
    if(MyKey == DOWN)
    {
      /* Set the Text Color */
      LCD_SetTextColor(White);
      LCD_DrawRect(IconRect[CurrentIcone][0], IconRect[CurrentIcone][1], 64, 65);
      
			do
			{
				CurrentIcone = (CurrentIcone+NbIconeByLine)%NumberOfIcons;
			}while( IconsAddr[CurrentIcone] [0] == NULL);
			
			/* if BT but no BT available on premium edition */
			if( CurrentIcone == 4 && !IsBTModulePresent())
			{
				do
				{
					CurrentIcone = (CurrentIcone+NbIconeByLine)%NumberOfIcons;
				}while( IconsAddr[CurrentIcone] [0] == NULL);
			}
			
    }
    /* If "LEFT" pushbutton is pressed */
    if(MyKey == LEFT)
    {
      /* Set the Text Color */
      LCD_SetTextColor(White);
      LCD_DrawRect(IconRect[CurrentIcone][0], IconRect[CurrentIcone][1], 64, 65);
			
			do
			{
				if ( CurrentIcone != 0)
					CurrentIcone = (CurrentIcone-1)%NumberOfIcons;
				else
					CurrentIcone = 11;
			}while( IconsAddr[CurrentIcone] [0] == NULL);
			
			/* if BT but no BT available on premium edition */
			if( CurrentIcone == 4 && !IsBTModulePresent())
			{
				do
				{
					if ( CurrentIcone != 0)
						CurrentIcone = (CurrentIcone-1)%NumberOfIcons;
					else
						CurrentIcone = 11;
				}while( IconsAddr[CurrentIcone] [0] == NULL);
			}
      
    }
    /* If "RIGHT" pushbutton is pressed */
    if(MyKey == RIGHT)
    {
      /* Set the Text Color */
      LCD_SetTextColor(White);
      LCD_DrawRect(IconRect[CurrentIcone][0], IconRect[CurrentIcone][1], 64, 65);
     
			do
			{
				CurrentIcone = (CurrentIcone+1)%NumberOfIcons;
			}while( IconsAddr[CurrentIcone] [0] == NULL);		

			/* if BT but no BT available on premium edition */
			if( CurrentIcone == 4 && !IsBTModulePresent())
			{
				do
				{
					CurrentIcone = (CurrentIcone+1)%NumberOfIcons;
				}while( IconsAddr[CurrentIcone] [0] == NULL);		
			}			
    }
		
		if( MyKey == RIGHT || MyKey == LEFT || MyKey == DOWN || MyKey == UP)
		{
			/* Set the Text Color */
      LCD_SetTextColor(Blue);
      LCD_DrawRect(IconRect[CurrentIcone][0], IconRect[CurrentIcone][1], 64, 65);

      /* Set the Back Color */
      LCD_SetBackColor(Blue);
      /* Set the Text Color */
      LCD_SetTextColor(White);
      /* Test on the MenuItemIndex value before incrementing it */
			MenuItemIndex = IconsAddr[CurrentIcone] [1];
			
      /* Get the current menu */
      psMenuItem = &(psCurrentMenu->psItems[MenuItemIndex]);
      LCD_DisplayStringLine(Line0, psMenuItem->pszTitle);
      ItemNumb[nMenuLevel] = MenuItemIndex;
		}
		
    /* If "SEL" pushbutton is pressed */
    if(MyKey == SEL)
    {
      SelFunc();			
      IntExtOnOffConfig(ENABLE);
      return;
    }
  }    
}

/**
  * @brief  This function is executed when any of "UP" push-butttons is pressed.
  * @param  None
  * @retval None
  */
void UpFunc(void)
{
	if(MenuActivated == false)
		return;
	
  /* Set the Back Color */
  LCD_SetBackColor(White);
  /* Set the Text Color */
  LCD_SetTextColor(Blue);
  psMenuItem = &psCurrentMenu->psItems[MenuItemIndex];
	LCD_DisplayStringLine(((MenuItemIndex + 1) * 2), psMenuItem->pszTitle);

  if(MenuItemIndex > 0)
  {
    MenuItemIndex--;
  }
  else
  {
    MenuItemIndex = psCurrentMenu->nItems - 1;
  }
  /* Set the Back Color */
  LCD_SetBackColor(Green);
  /* Set the Text Color */
  LCD_SetTextColor(White);
  psMenuItem = &psCurrentMenu->psItems[MenuItemIndex];
	LCD_DisplayStringLine(((MenuItemIndex + 1) * 2), psMenuItem->pszTitle);
  ItemNumb[nMenuLevel] = MenuItemIndex;
}

/**
  * @brief  This function is executed when any of "DOWN" push-butttons is pressed.
  * @param  None
  * @retval None
  */
void DownFunc(void)
{
	if(MenuActivated == false)
		return;
		
  /* Set the Back Color */
  LCD_SetBackColor(White);
  /* Set the Text Color */
  LCD_SetTextColor(Blue);
  psMenuItem = &psCurrentMenu->psItems[MenuItemIndex];
	LCD_DisplayStringLine(((MenuItemIndex + 1) * 2), psMenuItem->pszTitle);
      
  /* Test on the MenuItemIndex value before incrementing it */
  if(MenuItemIndex >= ((psCurrentMenu->nItems)-1))
  {
    MenuItemIndex = 0;
  }
  else
  {
    MenuItemIndex++;
  }
  /* Set the Back Color */
  LCD_SetBackColor(Green);
  /* Set the Text Color */
  LCD_SetTextColor(White);
  /* Get the current menu */
  psMenuItem = &(psCurrentMenu->psItems[MenuItemIndex]);
	LCD_DisplayStringLine(((MenuItemIndex + 1) * 2), psMenuItem->pszTitle);
  ItemNumb[nMenuLevel] = MenuItemIndex;
}

/**
  * @brief  This function is executed when "SEL" push-button is pressed.
  * @param  None
  * @retval None
  */
void SelFunc(void)
{
  psCurrentMenuItem = psMenuItem;

  if(psMenuItem->psSubMenu != '\0')
  {
    /* Update the current Item by the submenu */
    MenuItemIndex = 0;
    psCurrentMenu = psMenuItem->psSubMenu;
    psMenuItem = &(psCurrentMenu->psItems)[MenuItemIndex];
    DisplayMenu();
    nMenuLevel++;
    psPrevMenu[nMenuLevel] = psCurrentMenu;
  } 
  psCurrentMenuItem->pfMenuFunc();
}

/**
  * @brief  Initialize GPIO mapping of joystick
  * @param  None
  * @retval None
  */
void InitJoystick(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
       
   /* Enable BUTTON GPIO clock */
   RCC_APB2PeriphClockCmd( J_SEL_CLOCK 	|
                           J_DOWN_CLOCK	|
													 J_LEFT_CLOCK	|
													 J_RIGHT_CLOCK|
													 J_UP_CLOCK		, 
													 ENABLE );

	 /* Configure JOYSTICK BUTTON pins as input */
 
   /* JOYSTICK SEL Button */
   GPIO_InitStructure.GPIO_Pin   = J_SEL_PIN;
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
   GPIO_Init( J_SEL_PORT, &GPIO_InitStructure );

   /* JOYSTICK DOWN Button */
   GPIO_InitStructure.GPIO_Pin   = J_DOWN_PIN ;
   GPIO_Init( J_DOWN_PORT, &GPIO_InitStructure );

   /* JOYSTICK LEFT Button */
   GPIO_InitStructure.GPIO_Pin   = J_LEFT_PIN;
   GPIO_Init( J_LEFT_PORT, &GPIO_InitStructure );  

   /* JOYSTICK RIGHT Button */
   GPIO_InitStructure.GPIO_Pin   = J_RIGHT_PIN;
   GPIO_Init( J_RIGHT_PORT, &GPIO_InitStructure );
    
   /* JOYSTICK UP Button */
   GPIO_InitStructure.GPIO_Pin   = J_UP_PIN;
   GPIO_Init( J_UP_PORT, &GPIO_InitStructure );  
	 
	 /* for EXTI config */
	 
	 /* RIGHT Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);

  /* LEFT Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

  /* DOWN Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource7);

  /* UP Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

  /* SEL Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
