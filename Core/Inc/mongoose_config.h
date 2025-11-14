/*****************************************************************************
 * mongoose_config.h
 *
 *  Created on: Nov 14, 2025
 *   Copyright (C) 2022 Marco Solano (SOLANOLABS). All rights reserved.
 *   Author: Marco Solano <marco.solano@solanolabs.it>
 *
 *   Made for :
 *
 *****************************************************************************/
#ifndef INC_MONGOOSE_CONFIG_H_
#define INC_MONGOOSE_CONFIG_H_

  #ifdef __cplusplus
    extern "C" {
  #endif




	#define MG_ARCH 					MG_ARCH_NEWLIB
	#define MG_ENABLE_TCPIP				1
	#define MG_ENABLE_HTTP				1
	#define MG_ENABLE_POSIX_FS			0
	#define MG_ENABLE_CUSTOM_MILLIS		1
	//#define MG_ENABLE_CUSTOM_RANDOM		1
	#define MG_IO_SIZE					2048
	//#define MG_ENABLE_LOG=1
	#define MG_ENABLE_DRIVER_STM32H 	1  // Per la tua serie H7 (anche se usiamo custom WiFi, abilita HAL compatibilità)
	//#define MG_ENABLE_HTTP_URL_ENCODE	0  // Opzionale, riduce size se non serve URL encoding



  #ifdef __cplusplus
    }
  #endif
#endif
//INC_MONGOOSE_CONFIG_H_
