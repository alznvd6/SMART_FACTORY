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
#define FIRE_SENSOR_1_Pin GPIO_PIN_4
#define FIRE_SENSOR_1_GPIO_Port GPIOA
#define FIRE_SENSOR_2_Pin GPIO_PIN_5
#define FIRE_SENSOR_2_GPIO_Port GPIOA
#define BLUE_LED_FAN_Pin GPIO_PIN_0
#define BLUE_LED_FAN_GPIO_Port GPIOB
#define RED_LED_HEATER_Pin GPIO_PIN_1
#define RED_LED_HEATER_GPIO_Port GPIOB
#define WATER_SPLIT_SYS_Pin GPIO_PIN_2
#define WATER_SPLIT_SYS_GPIO_Port GPIOB
#define ALARM_BUZZER_Pin GPIO_PIN_3
#define ALARM_BUZZER_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
