/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifdef __STM32F4__

#endif
#include "stdio.h"
#include "m2m_types.h"


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* Exported macro ------------------------------------------------------------*/

/** Security mode */
#define USE_WEP					 (0) /*< Set to (1) to use WEP, and (0) to use OPEN */

/** AP mode Settings */
#define MAIN_WLAN_SSID           "DEMO_AP" /* < SSID */
#if USE_WEP
#define MAIN_WLAN_AUTH           M2M_WIFI_SEC_WEP /* < Security manner */
#define MAIN_WLAN_WEP_KEY        "1234567890" /* < Security Key in WEP Mode */
#define MAIN_WLAN_WEP_KEY_INDEX  (0)
#else
#define MAIN_WLAN_AUTH           M2M_WIFI_SEC_OPEN /* < Security manner */
#endif
#define MAIN_WLAN_CHANNEL        (6) /* < Channel number */


#ifndef NULL
#define NULL   ((void *) 0)
#endif

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IRQ_Pin_Pin GPIO_PIN_3
#define IRQ_Pin_GPIO_Port GPIOF
#define IRQ_Pin_EXTI_IRQn EXTI3_IRQn
#define WAKE_Pin_Pin GPIO_PIN_11
#define WAKE_Pin_GPIO_Port GPIOF
#define CS_Pin_Pin GPIO_PIN_14
#define CS_Pin_GPIO_Port GPIOD
#define RESET_Pin_Pin GPIO_PIN_2
#define RESET_Pin_GPIO_Port GPIOG
#define CHIP_En_Pin_Pin GPIO_PIN_3
#define CHIP_En_Pin_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
