/**
  @page Google Cloud IoT Core sample application

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of the Google Cloud IoT core application.
  ******************************************************************************
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. All rights reserved.
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  @endverbatim

@par Application Description

The Google Cloud IoT core application implements an MQTT client which connects to IoT core MQTT broker
in order to publish telemetry data and receive parameter updates or commands from the cloud.

The application connects to the broker thanks to the options and credentials provided by the user
on the console.

The application requires that user has created a Google Cloud Platform (GCP) account

Refer to the Google Cloud Platform, IoT Core, Pub/Sub documentation to learn how to define, configure
and create new project, registries, devices and topics that will be used with this STM32 application.

The device authenticates through JWT (Jason Web Token) as implemented by GCP IoT core

The list of available configuration options is detailed in the User Manual UM2441: X-CUBE-GCP.

It supports the B-L475E-IOT01, 32F413H-DISCO, 32F769I-DISCO and 32L496G-Discovery
boards and connects to the Internet through the on-board or attached network interface.

@par Hardware and Software environment


  - MCU board: STM32F769I-DISCO (MB1225 rev B)

  - Ethernet router.
      * With a transparent Internet connectivity: No proxy, no firewall blocking the outgoing traffic.
      * Running a DHCP server delivering the IP and DNS configuration to the board.

  - A development PC for building the application, programming through ST-Link, and running the virtual console.

  - A computer allowed to freely connect to the Internet.

  - An active Google Cloud Platform (GCP) account with configured GCP IoT Core entries. You can follow GCP IoT core
    documentation on how to configure the Cloud. This should somehow follow the steps below.
      * Create or select a GCP project. The project name will be used as {project-id} in connection string
      * Make sure that billing is enabled for the project
      * Enable Google Cloud IoT Core and Pub/Sub APIs
      * In IoT Core, create a new device registry, which will be defined as {registry-id} in connection string
          * select Region (by default us-central1), which can be later configured as {cloud-region} in connection string 
          * if not existing create a telemetry topic with name 'events' and keep default state topic.
           !!!! Take care to use 'events' as the topic name as this is the one used by the application !!!!
      * Next, in the newly created registry, add a new device:
          * Define a Device ID, which will be {device-id} in connection string.
          * As described in 'Creating Public/Private Key Pairs' documentation of GCP,
            use openssl for creating these public and private keys of type RS256 or ES256.
			In case of RS256:
                > openssl genrsa -out rsa_private.pem 2048
                > openssl rsa -in rsa_private.pem -pubout -out rsa_public.pem
			In case of ES256:
				> openssl ecparam -genkey -name prime256v1 -noout -out ec_private.pem
				> openssl ec -in ec_private.pem -pubout -out ec_public.pem
          * Add the public key pem content in the device public key menu of the cloud console
            and keep the private key to be passed later on at runtime to the board through
            the serial console interface.
      * Once done, your Cloud is setup for using and connecting your device to the GCP.
      * For checking proper connection, you can check the device details in the GCP console.
      * For visualizing telemetry data, one can use GCP SDK command line tool installed on the PC or create an application
        that will subscribe to pub/sub API. (Not detailed here)

@par How to use it ?

In order to make the program work, you must follow these steps:

Application build and flash
  - Open and build the project with one of the supported development toolchains (see the release note
    for detailed information about the version requirements).

  - Program the firmware on the STM32 board: you can copy (or drag and drop) the generated ELF
    file to the USB mass storage location created when you plug the STM32
    board to your PC. If the host is a Linux PC, the STM32 device can be found in
    the /media folder with the name e.g. "DIS_L4IOT". For example, if the created mass
    storage location is "/media/DIS_L4IOT", then the command to program the board
    with a binary file named "my_firmware.bin" is simply: cp my_firmware.bin
    /media/DIS_L4IOT.

   Alternatively, you can program the STM32 board directly through one of the
   supported development toolchains.


Application first launch

  - Connect the board to your Ethernet LAN.

  - Connect the board to your development PC through USB (ST-LINK USB port).
    Open the console through serial terminal emulator (e.g. TeraTerm), select the ST-LINK COM port of your
    board and configure it with:
    - 8N1, 115200 bauds, no HW flow control;
    - set the line endings to LF or CR-LF (Transmit) and LF (receive).

  - On the console:

    - Set the device connection string, without including the enclosing quotes (") or leading/trailing spaces.
      project-id=<my_project_ID>;cloud-region=<my_cloud_region>;registry-id=<my_registry_id>;device-id=<my_device_id>;

      When the device connects, it will use this information by setting the MQTT client ID to the full device path:
      projects/{project-id}/locations/{cloud-region}/registries/{registry-id}/devices/{device-id}

      Example of a connection string for connecting to gcloud server:
      project-id=stm32-iot-cloud-project;cloud-region=us-central1;registry-id=stm32-registry;device-id=1234-stm32-rs256;

      Important: Choose a unique ClientId value for your device ID.
                 For instance you can derive it from the MAC address of your board: c47f510111a7_is_great.

    - Set the TLS root CA certificates:

      Copy/paste the contents Projects/Common/GoogleIot/Comodo_google.pem

      This file contains 2 certificates. The device uses the first on to authenticate through TLS the HTTPS server which is
      used to retrieve the current time and date at boot time. The second one is used to authenticate through TLS the Google
      cloud server mqtt.googleapis.com. Note that this second root CA certificate might change over time or location, so you
      may need to check the list of supported CAs from pki.google.com/roots.pem.

    - Set the device Private Key:

      You must create a public/private key pair as described in GCP IoT core documentation.
      The public key shall be associated to the device in GCP IoT Core and the Private Key shall be passed to Device through console.
      ES256 and RS256 format are supported in this STM32 application for now (not RS256_X509 nor ES256_X509).

      Copy/paste the device Private Key to the console.

     - After the parameters are configured, it is possible to change them by restarting the board
     and pushing the User button or joystick (blue one) when prompted.


Application runtime
  - Makes an HTTPS request to retrieve the current time and date, and configures the RTC.

      Note: HTTPS has the advantage over NTP that the server can be authenticated by the board, preventing
            a possible man-in-the-middle attack.
            However, the first time the board is switched on (and each time it is powered down and up, if the RTC
            is not backed up), the verification of the server certificate will fail as its validity period will
            not match the RTC value.
            The following log will be printed to the console: "x509_verify_cert() returned -9984 (-0x2700)"
            This error is ignored, and the RTC is updated from the "Date:" field of the HTTP response header.

  - Connects to the GCP IoT Core MQTT broker.

  - Registers to the Device configuration topic /devices/{device-id}/config to get configuration commands from Cloud.

  - Publishes the status of the device on the topic /devices/{device-id}/state.

  - Stays idle, pending on local user, or cloud-initiated events.

    - Possible local user actions:

      - Single push on the user button or joystick:
            Trigs the sampling of the sensor values (or any basic application data like led status if there is no sensor)
            and the publication to the broker through an MQTT publish message to the "/devices/{device-id}/events"
            telemetry topic.

      - Double push on the user button or joystick:
            Starts or stops the values publication loop.
            When the loop is running, the sensor values (or simple any application data if no sensor) are published 
            every TelemetryInterval seconds.

      Note: Each publication is signaled by the user LED blinking quickly for half a second.

    - Possible cloud-initiated events, i.e. sending commands from Cloud to Device. The commands can be trigged in GCP 
      by defining new configurations 
      
      - From the Cloud console, in the IoT core user interface.
        Go to the Device registries page in GCP Console.
        Click the ID of the registry that contains the device.
        On the Registry details page, click the ID of the device whose configuration you want to update.
        At the top of the page, click Update config.
        Select a format for the configuration and paste the data in the Configuration box.
        Click Send to device.

      - On fimmware application side, the supported commands are:
        * LED control command:
          publish the payload {"LedOn": true} or {"LedOn": false}

        * Change of the Telemetry publication interval:
          publish the payload {"TelemetryInterval": <number of seconds>}

        * Reboot command:
          publish the payload {"Reboot": true}
          WARNING: the "config" is sent from Cloud to device at each new connection, so when sending {"reboot": true}, user should
          modify congig again and remove this command to avoid endless reboot. 

        * Accepted commands are:
            {"LedOn": true }
            {"LedOn": false }
            {"TelemetryInterval": <number of seconds> }
            {"Reboot": true }

        * The complete configuration can be used in GCP IoT Core device configuration interface.
            For instance:
            {
              "LedOn": false,
              "TelemetryInterval":60,
              "Reboot": false
            }

@par Directory contents

---> in .
Inc
  ethernetif.h                       Ethernet configuration for LWIP.
  flash.h                            Management of the internal flash memory.
  FreeRTOSConfig.h                   FreeRTOS configuration.
  lwipopts.h                         LWIP configuration.
  main.h                             Header containing config parameters for the application and globals.
  stm32f7xx_hal_conf.h               HAL configuration file.
  stm32f7xx_it.h                     STM32 interrupt handlers header file.

Src
  ethernetif.c                       Configuration of the ethernet interface.
  flash_f7.c                         Flash programming interface.
  main.c                             Main application file.
  stm32f7xx_hal_msp.c                Specific initializations.
  stm32f7xx_it.c                     STM32 interrupt handlers.
  system_stm32f7xx.c                 System initialization.

---> in Projects/Common/GoogleIoT
comodo_google.pem                     Root CA certificate to be pasted on the board console at first launch.

Inc
  googleiot.h                         Header file for googleiot.c
  googleiot_mbedtls_config.h          Header file with mbedtls configuration file for GCP iot project
  jwt.h                               Header file for jwt.c
  paho_mqtt_platform.h                Application-specific Paho middleware configuration

Src
  googleiot.c                         Application implementation
  jwt.c                               Jason Web Token utility implementation

---> in Projects/Common/Shared
Inc
  cloud.h                             Header for cloud.c
  heap.h                              Header for heap.c
  http_lib.h                          Header for http_lib.h
  iot_flash_config.h                  Header for iot_flash_config.c
  mbedtls_net.h                       Header for mbedtls_net.c
  msg.h                               Trace message interface
  net.h                               Net abstraction interface at transport layer level
  net_internal.h                      Net internal definitions
  paho_timer.h                        Header for paho_timer.c 
  rfu.h                               Header for rfu.c
  sensors_data.h                      Header for sensor_data.c
  timedate.h                          Header for timedate.c
  timingSystem.h                      Header for timingSystem.c
  version.h                           STM32 X-Cube package version definition

Src
  cloud.c
  entropy_hardware_poll.c             RNG entropy source for mbedTLS.
  heap.c                              Heap check functions
  http_lib.c                          Helpers for building HTTP requests, and downloading by chunks.
  iot_flash_config.c                  Dialog and storage management utils for the user-configured settings.
  lwip_net.c                          LwIP on ethernet init/deinit functions for NET (STM32F769I-DISCO).
  mbedtls_net.c                       Network adapter for mbedTLS on NET.
  net.c                               Network socket API.
  net_tcp_lwip.c                      NET TCP / LwIP on ethernet implementation.
  net_tls_mbedtls.c                   NET TLS / mbedTLS implementation.
  paho_timer.c                        Paho middleware timer wrapper.
  rfu.c                               Firmware versioning and change management.
  sensors_data.c                      Board-specific file to retrieve and format sensors data.
  STM32CubeRTCInterface.c             Libc time porting to the RTC.
  timedate.c                          Initialization of the RTC from the network.
  timingSystem.c                      Libc time porting to the RTC.

@Par Target-specific notes
  * The MAC address of the internet interface is derived from the unique CPU ID thanks to a SHA256 hash, imported from mbedTLS.
    If mbedTLS is not part of the project, any other hash function may be used, or unique MAC addresses assigned manually
    in stm32f7xx_hal_conf.h.

  * Due to the multiple definition of errno by IAR and LwIP in the Azure SDK, the following compilation switches are defined.
    Their values match LwIP errno.h values.
      EINVAL=22
      ENOMEM=12

  * The _SYS_TIME_H_ and _TIMEVAL_DEFINED compilation switches prevent a timestruct type redefinition from LwIP and the SystemWorkbench toolchain.
    Note: Defining LWIP_TIMEVAL_PRIVATE would be an alternative.

@par Caveats

  - The mbedTLS configuration parameter MBEDTLS_SSL_MAX_CONTENT_LEN is tailored down to 8kbytes.
    It commands the size of the TLS read buffer and of the TLS write buffer.
    It is sufficient for connecting to most MQTT brokers, and to the HTTPS server used for retrieving
    the time and date at boot time.
    But the TLS standard may require up to 16kbytes, depending on the server configuration.
    For instance, if the server certificate is 10kbytes large, it will not fit in the device 8kbytes buffer,
    the TLS handshake will fail, and the TLS connection will not be possible.

  - Beware the leading and trailing characters when entering the device connection string on the console.
    The string ends with the ';' separator.

@par Troubleshooting


  - Hardfault during the mbedTLS initialization
    * Check the contents of the root CA and device certificates and key configured by the user.

  - mbedTLS handshake failure
    * on mbedtls_pk_sign()
      - Undetected heap overflow.
    * On first recv()
      - Disconnection from the remote host because no common cipher was found (check the log of the server).

    * After sending the client hello, and before receiving the server hello
      - May happen if the hostname configured by the application through the "tls_server_name" socket option
        (and mbedtls_ssl_set_hostname()) does not match the configuration of the remote host.
        Typically, a remote HTTP server running several HTTP virtual hosts on the same port needs this Server Name Indication (SNI)
        field to select the TLS certificate to send to the client.

    * On server hello parsing
      - The server certificate may be larger than the mbedTLS read buffer (see @Caveats above).

  - mbedTLS read failure
    * A "bad message length" error message may signal that the received TLS chunk does not fit in the mbedTLS
      read buffer (see @Caveats above).


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */

