/**
  ******************************************************************************
  * @file    googleiot.c
  * @author  CLAB & MCD Application Team
  * @brief   Google Cloud IoT Core example.
  *          Basic telemetry on sensor-equipped boards.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "MQTTClient.h"
#include "cJSON.h"
#include "iot_flash_config.h"
#include "jwt.h"
#include "googleiot.h"
#include "version.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* The init/deinit netif functions are called from cloud.c.
 * However, the application needs to reinit whenever the connectivity seems to be broken. */
extern int net_if_reinit(void * if_ctxt);

/* Private defines -----------------------------------------------------------*/
#define g_hostname    "mqtt.googleapis.com"
#define g_port        "8883"

#define MODEL_MAC_SIZE                    13
#define MODEL_DEFAULT_MAC                 "0102030405"
#define MODEL_DEFAULT_LEDON               true
#define MODEL_DEFAULT_TELEMETRYINTERVAL   5
#define MAX_MQTT_ERRORS_BEFORE_RECONNECTION_NEEDED  3

#define MQTT_MSG_BUFFER_SIZE                  1024
#define MQTT_CMD_TIMEOUT                      5000
#define MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET  3

#define MQTT_TOPIC_BUFFER_SIZE            100  /**< Maximum length of the application-defined topic names. */

/* Private typedef -----------------------------------------------------------*/
typedef struct {
  char      mac[MODEL_MAC_SIZE];      /*< To be read from the netif */
  bool      LedOn;
  uint32_t  TelemetryInterval;
} status_data_t;

typedef struct
{
  char *HostName;
  char *HostPort;
  char *projectId;
  char *registryId;
  char *deviceId;
  char *cloudRegion;
} gcp_config_t;


typedef struct
{
  Network net;    /** Network related information */
  MQTTClient client;     /** MQTT client */
  gcp_config_t cfg;   /** Configuration options */
  unsigned char buf[MQTT_MSG_BUFFER_SIZE];
  unsigned char readbuf[MQTT_MSG_BUFFER_SIZE];
} gcp_client_t;

/* Private macros ------------------------------------------------------------*/
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

/* Private variables ---------------------------------------------------------*/
static bool g_continueRunning;
static bool g_publishData;
static bool g_statusChanged;
static bool g_reboot;
static int g_connection_needed_score;
status_data_t status_data = { MODEL_DEFAULT_MAC, MODEL_DEFAULT_LEDON, MODEL_DEFAULT_TELEMETRYINTERVAL };

/* Warning: The subscribed topics names strings must be allocated separately,
 * because Paho does not copy them and uses references to dispatch the incoming message. */
static char mqtt_subtopic[MQTT_TOPIC_BUFFER_SIZE];
static char mqtt_msg[MQTT_MSG_BUFFER_SIZE];

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
int GcpIoT_configure(gcp_config_t *options);
int GcpIoT_connect(gcp_client_t *gcpClient);
int GcpIoT_publishTelemetry(gcp_client_t *gcpClient);
int GcpIoT_publishDeviceState(gcp_client_t *gcpClient);
int GcpIoT_subscribe( gcp_client_t *gcpClient );
int GcpIoT_disconnect(gcp_client_t *gcpClient);
int32_t comp_left_ms(uint32_t init, uint32_t now, uint32_t timeout);
void free_device_config(gcp_config_t * config);
void allpurposeMessageHandler(MessageData* data);

#ifdef SENSOR
static void read_sensors(float *TEMPERATURE_Value, float *HUMIDITY_Value, float *PRESSURE_Value)
{
  *TEMPERATURE_Value = BSP_TSENSOR_ReadTemp();
  *HUMIDITY_Value = BSP_HSENSOR_ReadHumidity();
  *PRESSURE_Value = BSP_PSENSOR_ReadPressure();
}
#endif /* SENSOR */

/** Function to read data from the socket opened into provided buffer
 * @param - Address of Network Structure
 *        - Buffer to store the data read from socket
 *        - Expected number of bytes to read from socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes read on SUCCESS
 *         - -1 on FAILURE
 **/
static int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
  int bytes;

  bytes = net_sock_recv((net_sockhnd_t) n->my_socket, buffer, len);
  if(bytes < 0)
  {
    msg_error("net_sock_recv failed - %d\n", bytes);
    bytes = -1;
  }

   return bytes;
}

/** Function to write data to the socket opened present in provided buffer
 * @param - Address of Network Structure
 *        - Buffer storing the data to write to socket
 *        - Number of bytes of data to write to socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
static int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
  int rc;

  rc = net_sock_send((net_sockhnd_t) n->my_socket, buffer, len);
  if(rc < 0)
  {
    msg_error("net_sock_send failed - %d\n", rc);
    rc = -1;
  }

  return rc;
}

/** Message callback
 *
 *  Note: No context handle is passed by the callback. Must rely on static variables.
 *        TODO: Maybe store couples of hander/contextHanders so that the context could
 *              be retrieved from the handler address. */
void allpurposeMessageHandler(MessageData* data)
{
  snprintf(mqtt_msg, MIN(MQTT_MSG_BUFFER_SIZE, data->message->payloadlen + 1),
           "%s", (char *)data->message->payload);
  msg_info("Received message: topic: %.*s content: %s.\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           mqtt_msg);

  cJSON *json = NULL;
  cJSON * root = cJSON_Parse(mqtt_msg);

  json = cJSON_GetObjectItemCaseSensitive(root, "LedOn");
  if (json != NULL)
  {
    if (cJSON_IsBool(json) == true)
    {
      status_data.LedOn = (cJSON_IsTrue(json) == true);
      Led_SetState(status_data.LedOn);
      g_statusChanged = true;
    }
    else
    {
      msg_error("JSON parsing error of LedOn value.\n");
    }
  }

  json = cJSON_GetObjectItemCaseSensitive(root, "TelemetryInterval");
  if (json != NULL)
  {
    if (cJSON_IsNumber(json) == true)
    {
      status_data.TelemetryInterval = json->valueint;
      g_statusChanged = true;
    }
    else
    {
      msg_error("JSON parsing error of TelemetryInterval value.\n");
    }
  }

  json = cJSON_GetObjectItemCaseSensitive(root, "Reboot");
  if (json != NULL)
  {
    if (cJSON_IsBool(json) == true)
    {
      g_reboot = (cJSON_IsTrue(json) == true);
    }
    else
    {
      msg_error("JSON parsing error of Reboot value.\n");
    }
  }

  cJSON_Delete(root);
}

int cloud_device_enter_credentials(void)
{
  iot_config_t iot_config;
  int ret = 0;

  memset(&iot_config, 0, sizeof(iot_config_t));

  msg_info("\n");

  msg_info("\nEnter the Google Cloud IoT Core connection string of your device: (template: %s=xxx;%s=xxx;%s=xxx;%s=xxx) \n", PROJECT_ID_KEY, REGISTRY_ID_KEY, DEVICE_ID_KEY, CLOUD_REGION_KEY);

  getInputString(iot_config.device_name, USER_CONF_DEVICE_NAME_LENGTH);

  msg_info("connection string: --->\n%s\n<---\n", iot_config.device_name); 

  if(setIoTDeviceConfig(&iot_config) != 0)
  {
    ret = -1;
    msg_error("Failed programming the IoT device configuration to Flash.\n");
  }

  return ret;
}

bool app_needs_device_keypair()
{
  return true;
}

bool app_needs_root_ca(void)
{
  return true;
}

/* Helper function to allocate memory while copying content */
static  void strCopy(char **dest, char *src){
  if (src == NULL)
  {
    *dest = NULL;
    return;
  }
  if(strlen(src) >= 1){
    *dest = (char*)malloc(sizeof(char)*(strlen(src)+1));
    strcpy(*dest,src);
  }
}

/**
  * @brief  Allocate and return a gcp_config_t structure initialized with the values defined by the passed configuration string. The buffers holding the structure and those fields are allocated dynamically allocated.
  * @param  Out: options       pointer to the Google Cloud ioT core device info structure
  * @retval 0:    the connection string in Flash has been parsed succesfully and the output structure fields have been set
  *        -1:    in case of failure
  */
int GcpIoT_configure( gcp_config_t * options )
{
  char ** devInfoPtr = NULL;           /* pointer toward the values in the output structure */
  const char * config_string=NULL;           /* pointer to configuration string */
  char workCfg[USER_CONF_DEVICE_NAME_LENGTH]; /* local variable receiving the configuration string (or device name) copy */
  const char s[] = ";=";                /* delimiter used in the string to parse */
  char * smallerStr;                   /* pointer to smaller strings that are included in the device name string */
  int ret = 0;

  /* initialising the output structure */
  memset(options, 0, sizeof(gcp_config_t));
  strCopy(&options->HostName,g_hostname);
  strCopy(&options->HostPort,g_port);

  /* initialising the local array */
  memset(&workCfg, 0, sizeof(workCfg));

  /* retrieve the device name address in flash */
  getIoTDeviceConfig(&config_string);

  /* copy its content into the local variable */
  strcpy(workCfg,config_string);

  /* get the first smaller string */
  smallerStr = strtok(workCfg, s);

  /* walk through other smaller strings */
  while( (smallerStr != NULL) && (ret == 0) )
  {
    msg_debug( "connection string part: %s\n", smallerStr );

    if (strcmp(smallerStr, PROJECT_ID_KEY) == 0)
    {
       devInfoPtr = &options->projectId;
    }
    else if (strcmp(smallerStr, REGISTRY_ID_KEY) == 0)
    {
      devInfoPtr = &options->registryId;
    }
    else if (strcmp(smallerStr, DEVICE_ID_KEY) == 0)
    {
      devInfoPtr = &options->deviceId;
    }
    else if (strcmp(smallerStr, CLOUD_REGION_KEY) == 0)
    {
      devInfoPtr = &options->cloudRegion;
    }
    else
    {
      if (devInfoPtr != NULL)
      {
        strCopy(devInfoPtr,smallerStr);
        /* this pointer shall be set again when a key word is found */
        devInfoPtr = NULL;
      }
      else
      {
        ret = -1;
        msg_info( "Your Google Cloud IoT Core connection string cannot be parsed correctly.\nPlease check its syntax and reprogram it pressing the user button (blue) during the initialisation phase.\n" );
      }
    }

    if (ret == 0)
    {
      smallerStr=strtok(NULL, s);
    }
  } /* while end */

  return ret;
}

/** Free a device_config_t allocated by parse_and_fill_device_config().
 */
void free_device_config(gcp_config_t * config)
{
  if (config != NULL)
  {
    if (config->HostName != NULL) free(config->HostName);
    if (config->HostPort != NULL) free(config->HostPort);
    if (config->projectId != NULL) free(config->projectId);
    if (config->registryId != NULL) free(config->registryId);
    if (config->deviceId != NULL) free(config->deviceId);
    if (config->cloudRegion != NULL) free(config->cloudRegion);
  }
  else
  {
    msg_warning("Attemped to free a non-allocated config structure.\n");
  }
}

void gcp_main(void const *arg)
{
  int ret = 0;
  gcp_client_t gcpClient;

  g_continueRunning = true;
  g_publishData     = false;
  /* Publish state at least one just after connection */
  g_statusChanged   = true;
  g_reboot          = false;
  g_connection_needed_score = 0;

  GcpIoT_configure(&gcpClient.cfg);

  /* Initialize the defaults of the published messages. */
  net_macaddr_t mac = { 0 };
  if (net_get_mac_address(hnet, &mac) == NET_OK)
  {
    snprintf(status_data.mac, MODEL_MAC_SIZE, "%02X%02X%02X%02X%02X%02X",
             mac.mac[0], mac.mac[1], mac.mac[2], mac.mac[3], mac.mac[4], mac.mac[5]);
  }
  else
  {
    msg_warning("Could not retrieve the MAC address to set the device ID.\n");
    snprintf(status_data.mac, MODEL_MAC_SIZE, "UnknownMAC");
  }

  do {
    /* Init MQTT client */
    net_ipaddr_t ip;

    /* If the socket connection failed MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET times in a row,
    * even if the netif still has a valid IP address, we assume that the network link is down
    * and must be reset. */
    if ( (net_get_ip_address(hnet, &ip) == NET_ERR) || (g_connection_needed_score == MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET) )
    {
      msg_info("Network link %s down. Trying to reconnect.\n", (g_connection_needed_score == MAX_SOCKET_ERRORS_BEFORE_NETIF_RESET) ? "may be" : "");
      if (net_reinit(hnet, (net_if_reinit)) != 0)
      {
        msg_error("Netif re-initialization failed.\n");
        break;
      }
      else
      {
        msg_info("Netif re-initialized successfully.\n");
        HAL_Delay(1000);
      }
    }

    /* Connect to cloud */
    ret = GcpIoT_connect(&gcpClient);

    /* If connection was successful, also subscribe to device configuration topic */
    if (ret == NET_OK) {
      ret = GcpIoT_subscribe(&gcpClient);
    }

    if (ret != NET_OK)
    {
      msg_error("Could not open the socket at %s port %d.\n", gcpClient.cfg.HostName, atoi(gcpClient.cfg.HostPort));
      g_connection_needed_score = MAX_MQTT_ERRORS_BEFORE_RECONNECTION_NEEDED;
      HAL_Delay(1000);
    }
    else
    {
#ifdef SENSOR
      printf("When user button is pushed shortly, application publishes the sensor values.\n");
#else
      printf("When user button is pushed shortly, application publishes a 0/1 toggle value.\n");
#endif
      printf("On double button push, application enters in a publication loop. Next double push returns to previous mode\n\n");

      /* Send the telemetry data, and send the device status if it was changed by a received message. */
      uint32_t last_telemetry_time_ms = HAL_GetTick();
      g_connection_needed_score = 0;
      do
      {
        uint8_t command = Button_WaitForMultiPush(500);
        bool b_sample_data = (command == BP_SINGLE_PUSH); /* If short button push, publish once. */
        if (command == BP_MULTIPLE_PUSH)                  /* If long button push, toggle the telemetry publication. */
        {
          g_publishData = !g_publishData;
          msg_info("%s the sensor values publication loop.\n", (g_publishData == true) ? "Enter" : "Exit");
        }

        int32_t left_ms = comp_left_ms(last_telemetry_time_ms, HAL_GetTick(), status_data.TelemetryInterval * 1000);

        if ( ((g_publishData == true) && (left_ms <= 0))
            || (b_sample_data == true) )
        {
          /* build and publish */
          if ( (ret < 0) || (ret >= MQTT_MSG_BUFFER_SIZE) )
          {
            msg_error("Telemetry message formatting error.\n");
          }
          else
          {
            /* Point in main loop that is periodically reached at Telemetry interval
             * during an active connection to Cloud
             */
            int ret = 0;
            /* Publish the Telemetry data */
            ret = GcpIoT_publishTelemetry(&gcpClient);
            if (ret == MQSUCCESS)
            {
              /* Visual notification of the telemetry publication: LED blink. */
              Led_Blink(80, 40, 5);
              /* Restore the LED state */
              Led_SetState(status_data.LedOn);
              last_telemetry_time_ms = HAL_GetTick();
            }
            else
            {
              msg_error("Telemetry publication failed. Reconnection needed?\n");
              g_connection_needed_score++;
            }
          }
        }

        /* Publish the updated device status if it changed */
        if (g_statusChanged)
        {
          ret = GcpIoT_publishDeviceState(&gcpClient);
          if (ret != MQSUCCESS)
          {
            msg_error("Publish state failed. Reconnection needed?\n");
            g_connection_needed_score++;
          }
        }

        ret = MQTTYield(&gcpClient.client, 500);
        if (ret != MQSUCCESS)
        {
          msg_error("Yield failed. Reconnection needed?\n");
          g_connection_needed_score++;
        }
        else
        {
          msg_info(".");
        }
      } while ( g_continueRunning && !g_reboot && (g_connection_needed_score < MAX_MQTT_ERRORS_BEFORE_RECONNECTION_NEEDED) );

      /* The publication loop is exited.
      NB: No need to unsubscribe as we are disconnecting. */
      GcpIoT_disconnect(&gcpClient);

      if (NET_OK !=  net_sock_close(gcpClient.net.my_socket))
      {
        msg_error("net_sock_close() failed.\n");
      }
    }

    if (NET_OK != net_sock_destroy(gcpClient.net.my_socket))
    {
      msg_error("net_sock_destroy() failed.\n");
    }
  } while (!g_reboot && (g_connection_needed_score > 0));

  free_device_config(&gcpClient.cfg);

  if(g_reboot == true)
  {
    platform_deinit();
    msg_info("Calling HAL_NVIC_SystemReset()\n");
    HAL_NVIC_SystemReset();
  }
}

int GcpIoT_connect( gcp_client_t *gcpClient )
{
  int ret;
  unsigned short keepAliveInterval = 60;
  char clientId[512];
  unsigned char jwt_token[1024];
  net_sockhnd_t socket;
  const char * ca_cert      = NULL;
  const char * device_key   = NULL;
  
  printf("Creating socket... ");
  ret = net_sock_create(hnet, &socket, NET_PROTO_TLS);
  if (ret != NET_OK)
  {
    msg_error("Could not create the socket.\n");
    return -1;
  }
  printf("done.\r\n");

  ret |= (checkTLSRootCA() != 0)
    || (getTLSKeys(&ca_cert, NULL, &device_key) != 0);
  ret |= net_sock_setopt(socket, "tls_server_name", (void *) gcpClient->cfg.HostName, strlen(gcpClient->cfg.HostName) + 1);
  ret |= net_sock_setopt(socket, "tls_ca_certs",    (void *) ca_cert, strlen(ca_cert) + 1);

  ret |= net_sock_setopt(socket, "sock_noblocking", NULL, 0);

  if (ret != NET_OK)
  {
    msg_error("Could not retrieve the security connection settings and set the socket options.\n");
    return -1;
  }

  printf("Creating jwt... ");
  ret = create_jwt( jwt_token, sizeof(jwt_token), (int) time(NULL), gcpClient->cfg.projectId, (const unsigned char *) device_key, strlen(device_key) + 1 );
  if (ret != JWT_OK)
  {
    msg_error("JWT creation failed, error=%d\n", ret);
    return -1;
  }
  printf("done.\r\n");

  printf("Opening socket... ");
  ret = net_sock_open(socket, gcpClient->cfg.HostName, atoi(gcpClient->cfg.HostPort), 0);
  if (ret != NET_OK)
  {
    msg_error("Could not open the socket at %s port %s.\n", gcpClient->cfg.HostName, gcpClient->cfg.HostPort);
    return -1;
  }
  printf("done.\r\n");

  gcpClient->net.my_socket = socket;
  gcpClient->net.mqttread = (network_read);
  gcpClient->net.mqttwrite = (network_write);

  MQTTClientInit(&gcpClient->client, &gcpClient->net, MQTT_CMD_TIMEOUT,
             gcpClient->buf, MQTT_MSG_BUFFER_SIZE,
             gcpClient->readbuf, MQTT_MSG_BUFFER_SIZE);

  snprintf(clientId, sizeof(clientId),"projects/%s/locations/%s/registries/%s/devices/%s",
          gcpClient->cfg.projectId, gcpClient->cfg.cloudRegion,
          gcpClient->cfg.registryId, gcpClient->cfg.deviceId );

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.willFlag = 0;
  data.MQTTVersion = 4;
  data.clientID.cstring = clientId;
  data.keepAliveInterval = keepAliveInterval;
  data.cleansession = 1;
  data.username.cstring="unused";
  data.password.cstring=(char*)jwt_token;

  printf("Opening MQTT connection to client %s\n", clientId);

  ret=MQTTConnect(&gcpClient->client, &data);

  if ( ret == MQSUCCESS )
  {
    printf("MQTT Connection ok\r\n");
    return 0;
  }
  else
  {
    printf("MQTT Connection failure... retrying... ");
    return -1;
  }
}

/** Publish topic to the cloud. If sensor data is
  * available, it will be published.
  */
int GcpIoT_publishTelemetry( gcp_client_t *gcpClient )
{
  int rc;
  char publishTopic[64];
  char payload[512];
  char tempbuf[16];
  MQTTMessage pub;
  static long int i = 0;
#ifndef SENSOR
  static int toggle = 0;
#endif

  sprintf(publishTopic,"/devices/%s/events", gcpClient->cfg.deviceId);

  pub.qos = QOS1;
  pub.retained = '0';
  pub.payload = (void*)payload;

#ifdef SENSOR
  float sensorT,sensorH,sensorP;
  read_sensors(&sensorT,&sensorH,&sensorP);
#else
  /* Send Toggle value */
  toggle = 1 - toggle;
#endif /* SENSOR */

  strcpy((char *)payload,"\"data\":[{");
#ifdef SENSOR
  strcat((char *)payload,"\"dataType\":\"temp\",\"value\":");
  sprintf(tempbuf,"\"%.2f\"",sensorT);
  strcat((char *)payload,tempbuf);

  strcat((char *)payload,"},{");
  strcat((char *)payload,"\"dataType\":\"hum\",\"value\":");
  sprintf(tempbuf,"\"%.2f\"",sensorH);
  strcat((char *)payload,tempbuf);

  strcat((char *)payload,"},{");
  strcat((char *)payload,"\"dataType\":\"pres\",\"value\":");
  sprintf(tempbuf,"\"%.2f\"",sensorP);
  strcat((char *)payload,tempbuf);
#else /* SENSOR */
  strcat((char *)payload,"\"dataType\":\"Toggle\",\"value\":");
  sprintf(tempbuf,"\"%d\"",toggle);
  strcat((char *)payload,tempbuf);
#endif
  strcat((char *)payload,"}]}");

  pub.payloadlen = strlen(payload);

  rc = MQTTPublish(&gcpClient->client, publishTopic, &pub);

  if(rc == MQSUCCESS)
  {
    msg_info("#%lu publication topic: %s \tpayload: %s\n", i, publishTopic, (char *)(pub.payload));
    i++;
  }
  else
  {
    printf("MQTT connection lost, rc=%d\r\n", rc);
    msg_error("Failed publishing %s on %s\n", (char *)(pub.payload), publishTopic);
  }
  return rc;
}

/** Publish device state to the cloud.
  */
int GcpIoT_publishDeviceState(gcp_client_t *gcpClient)
{
  int ret;
  char publishTopic[64];
  char payload[512];
  MQTTMessage pub;

  ret = sprintf(publishTopic,"/devices/%s/state", gcpClient->cfg.deviceId);

  pub.qos = QOS1;
  pub.retained = '0';
  pub.payload = (void*)payload;

  ret = snprintf( payload, MQTT_MSG_BUFFER_SIZE, "{\n"
                     "  \"LedOn\": %s,\n"
                     "  \"TelemetryInterval\": %d,\n"
                     "  \"ts\": %ld,\n"
                     "  \"mac\": \"%s\",\n"
                     "  \"FW version\": \"V%d.%d.%d\"\n"
                     "}",
                     (status_data.LedOn == true) ? "true" : "false",
                     (int) status_data.TelemetryInterval,
                     (long) time(NULL),
                     status_data.mac,
                     (int) FW_VERSION_MAJOR,
                     (int) FW_VERSION_MINOR,
                     (int) FW_VERSION_PATCH );

  pub.payloadlen = strlen(payload);

  printf("Publish Device State:\n%s\n", payload);

  if (ret < 0)
  {
    msg_error("Device State formatting error.\n");
    return -1;
  }
  ret = MQTTPublish(&gcpClient->client, publishTopic, &pub);
  if (ret != MQSUCCESS)
  {
    msg_error("Err: %d, Failed publishing %s on %s\n", ret, (char *)(pub.payload), publishTopic);
    return -1;
  }

  ret = MQTTYield(&gcpClient->client, 1000);
  if (ret != MQSUCCESS)
  {
    msg_error("Yield failed.\n");
    return -1;
  }

  g_statusChanged = false;
  return 0;
}

/** Subscribe to device configuration topic , this is how device
  * can get ocnfiguration data (LED state, Telemetry interval ..)
  * from Cloud.
  */
int GcpIoT_subscribe( gcp_client_t *gcpClient )
{
  int ret;

  /* Now subscribe to Device Configuration */
  snprintf(mqtt_subtopic, MQTT_TOPIC_BUFFER_SIZE, "/devices/%s/config", gcpClient->cfg.deviceId);
  ret = MQTTSubscribe(&gcpClient->client, mqtt_subtopic, QOS1, (allpurposeMessageHandler));

  if (ret != MQSUCCESS)
  {
    msg_error("Failed subscribing to the %s topic.\n", mqtt_subtopic);
    return -1;
  }

  ret = MQTTYield(&gcpClient->client, 1000);
  if (ret != MQSUCCESS)
  {
    msg_error("Yield failed.\n");
    return -1;
  }

  msg_info("Subscribed to %s.\n", mqtt_subtopic);
  return 0;
}

int GcpIoT_disconnect( gcp_client_t *gcpClient )
{
  /* NB: GcpIoT_disconnect() may raise additional error logs if the network link is already broken,
     but it is the only way to clean the MQTT session. */
  int rc = MQTTDisconnect(&gcpClient->client);
  if ( rc == MQSUCCESS )
  {
    printf("MQTT Disconnection ok\r\n");
  }
  else
  {
    printf("MQTT Disconnection failure\r\n");
  }
  return rc;
}

/**
 * @brief   Return the integer difference between 'init + timeout' and 'now'.
 *          The implementation is robust to uint32_t overflows.
 * @param   In:   init      Reference index.
 * @param   In:   now       Current index.
 * @param   In:   timeout   Target index.
 * @retval  Number of units from now to target.
 */
int32_t comp_left_ms(uint32_t init, uint32_t now, uint32_t timeout)
{
  int32_t ret = 0;
  uint32_t wrap_end = 0;

  if (now < init)
  { /* Timer wrap-around detected */
    /* printf("Timer: wrap-around detected from %d to %d\n", init, now); */
    wrap_end = UINT32_MAX - init;
  }
  ret = wrap_end - (now - init) + timeout;

  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
