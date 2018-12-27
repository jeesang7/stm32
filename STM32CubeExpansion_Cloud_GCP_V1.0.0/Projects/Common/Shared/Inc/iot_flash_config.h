/**
  ******************************************************************************
  * @file    iot_flash_config.h
  * @author  MCD Application Team
  * @brief   Header for configuration
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

#ifndef iot_flash_config_H
#define iot_flash_config_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Please note that this file does not include "main.h" so you need to define switches like AZURE_OTA at project level if needed */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define USER_CONF_C2C_SOAPC_MAX_LENGTH  16
#define USER_CONF_C2C_USERID_MAX_LENGTH 16
#define USER_CONF_C2C_PSW_MAX_LENGTH    16

#define USER_CONF_WIFI_SSID_MAX_LENGTH  32
#define USER_CONF_WIFI_PSK_MAX_LENGTH   64

#define USER_CONF_DEVICE_NAME_LENGTH    300   /**< Must be large enough to hold a complete configuration string */
#define USER_CONF_SERVER_NAME_LENGTH    128
#define USER_CONF_TLS_OBJECT_MAX_SIZE   2048
#define USER_CONF_MAGIC                 0x0123456789ABCDEFuLL

/* Azure Firmware Version Max. Len */
#define IOT_STATE_FW_VERSION_LEN        64

typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  bool use_internal_sim;                              /**< SIM slot selector. */
  char oper_ap_code[USER_CONF_C2C_SOAPC_MAX_LENGTH];  /**< C2C sim operator access point code. */
  char username[USER_CONF_C2C_USERID_MAX_LENGTH];     /**< C2C username. */
  char password[USER_CONF_C2C_PSW_MAX_LENGTH];        /**< C2C password. */

} c2c_config_t;

typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  char ssid[USER_CONF_WIFI_SSID_MAX_LENGTH];          /**< Wifi network SSID. */
  char psk[USER_CONF_WIFI_PSK_MAX_LENGTH];            /**< Wifi network PSK. */
  uint8_t security_mode;                              /**< Wifi network security mode. See @ref wifi_network_security_t definition. */
} wifi_config_t;

/* Bluemix key words */
#ifdef BLUEMIX
#define QUICK_START_REG_NAME "QuickStart"
#define SIMPLE_REG_NAME      "SimpleReg"
#define ORG_ID_KEY           "OrgId"
#define DEVICE_TYPE_KEY      "DeviceType"
#define DEVICE_ID_KEY        "DeviceId"
#define TOKEN_KEY            "Token"
#define IBM_IOT_URL          "internetofthings.ibmcloud.com"
#endif

#ifdef GOOGLEIOT
#define PROJECT_ID_KEY       "project-id"
#define REGISTRY_ID_KEY      "registry-id"
#define DEVICE_ID_KEY        "device-id"
#define CLOUD_REGION_KEY     "cloud-region"
#define USER_CONF_HTTPKEY_LENGTH         64
#define USER_CONF_HTTPSECRET_LENGTH     128
#endif

typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  char device_name[USER_CONF_DEVICE_NAME_LENGTH];
  char server_name[USER_CONF_SERVER_NAME_LENGTH];
#ifdef GOOGLEIOT
  char device_HttpKey[USER_CONF_HTTPKEY_LENGTH];
  char device_HttpSecret[USER_CONF_HTTPSECRET_LENGTH];
#endif
} iot_config_t;

#if defined(AZURE_OTA)
typedef struct {
  uint64_t magic;                                     /**< The USER_CONF_MAGIC magic word signals that the structure was once written to FLASH. */
  char prev_fw_version[IOT_STATE_FW_VERSION_LEN];     /**< Azure OTA: version of the Firmware which processed the FOTA request                  */
  uint8_t fota_running;                              /**< Azure OTA: installation procedure in progress or not                                 */
} iot_state_t;
#endif /* AZURE_OTA */

/** Static user configuration data which must survive reboot and firmware update.
 * Do not change the field order, due to firewall constraint the tls_device_key size must be placed at a 64 bit boundary.
 * Its size must also be multiple of 64 bits.
 *
 * Depending on the available board peripherals, the c2c_config and wifi_config fields may not be used.
 */
typedef struct {
  char tls_root_ca_cert[USER_CONF_TLS_OBJECT_MAX_SIZE * 3]; /* Allow room for 3 root CA certificates */
#ifndef GOOGLEIOT
  char tls_device_cert[USER_CONF_TLS_OBJECT_MAX_SIZE];
#endif
  /* Above structure member must be aligned on 256 byte boundary
     to match firewall constraint , same for the size. */
  char tls_device_key[USER_CONF_TLS_OBJECT_MAX_SIZE];
#ifdef USE_C2C
  c2c_config_t c2c_config;
#endif
#ifdef USE_WIFI
  wifi_config_t wifi_config;
#endif
  iot_config_t iot_config;
#if defined(AZURE_OTA)
  iot_state_t iot_state;        /**< generic iot_state which can be used to add OTA-related information for instance (Azure) */
#endif /* AZURE_OTA */
  uint64_t ca_tls_magic;        /**< The USER_CONF_MAGIC magic word signals that the TLS root CA certificates strings
                                    (tls_root_ca_cert) are present in Flash. */
  uint64_t device_tls_magic;    /**< The USER_CONF_MAGIC magic word signals that the TLS device certificate and key
                                    (tls_device_cert and tls_device_key) are present in Flash. */
} user_config_t;

int enterPemString(char * read_buffer, size_t max_len);
int getInputString(char *inputString, size_t len);

int checkC2cCredentials(const char ** const oper_ap_code, const char ** const username, const char ** const password, bool * use_internal_sim);
int updateC2cCredentials(void);
int checkWiFiCredentials(const char ** const ssid, const char ** const psk, uint8_t * const security_mode);
int updateWiFiCredentials(void);

int updateTLSCredentials(void);
int checkTLSRootCA(void);
int checkTLSDeviceConfig(void);
int getTLSKeys(const char ** const ca_cert, const char ** const device_cert, const char ** const private_key);

int setIoTDeviceConfig(iot_config_t *config);
int getIoTDeviceConfig(const char ** const name);
int checkIoTDeviceConfig(void);

#ifdef AWS
int getServerAddress(const char ** const address);
#endif /* AWS */

int updateFirmwareVersion(void);

#if defined(AZURE_OTA)
int setIoTState(iot_state_t *state);
int getIoTState(const iot_state_t **state);
#endif /* AZURE_OTA */

#ifdef __cplusplus
}
#endif
#endif /* iot_flash_config_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

