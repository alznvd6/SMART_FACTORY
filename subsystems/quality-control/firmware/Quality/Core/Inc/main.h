/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WEIGHT_Pin GPIO_PIN_0
#define WEIGHT_GPIO_Port GPIOA
#define COLOR_Pin GPIO_PIN_1
#define COLOR_GPIO_Port GPIOA
#define DATA_TX_Pin GPIO_PIN_2
#define DATA_TX_GPIO_Port GPIOA
#define DATA_RX_Pin GPIO_PIN_3
#define DATA_RX_GPIO_Port GPIOA
#define MOTION_Pin GPIO_PIN_4
#define MOTION_GPIO_Port GPIOA
#define MOTION_EXTI_IRQn EXTI4_IRQn
#define ON_OFF_Pin GPIO_PIN_5
#define ON_OFF_GPIO_Port GPIOA
#define ON_OFF_EXTI_IRQn EXTI9_5_IRQn
#define RELAY_REV_Pin GPIO_PIN_7
#define RELAY_REV_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_0
#define LED_GREEN_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_1
#define LED_RED_GPIO_Port GPIOB
#define D7_Pin GPIO_PIN_10
#define D7_GPIO_Port GPIOB
#define RS_Pin GPIO_PIN_11
#define RS_GPIO_Port GPIOB
#define E_Pin GPIO_PIN_12
#define E_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_13
#define D4_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_14
#define D5_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_15
#define D6_GPIO_Port GPIOB
#define RELAY_FWD_Pin GPIO_PIN_8
#define RELAY_FWD_GPIO_Port GPIOA
#define US_TX_Pin GPIO_PIN_9
#define US_TX_GPIO_Port GPIOA
#define US_RX_Pin GPIO_PIN_10
#define US_RX_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
