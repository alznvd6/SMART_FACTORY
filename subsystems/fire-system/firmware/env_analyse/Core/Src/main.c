/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define BUFFER_SIZE 10
#define ALARM_HOLD_TIME_MS 5000 // 5 seconds lock-out

typedef enum {
    SYSTEM_IDLE = 0,
    SYSTEM_COOLING,  // Fan Active
    SYSTEM_HEATING   // Heater Active
} SystemState_t;


typedef struct {
    float buffer[BUFFER_SIZE];
    int index;
    int is_populated;
    float target_temp;
    float tolerance_percent;
    SystemState_t state;

    // --- Fire System Additions ---
    int fire_detected;         // Flag if active fire is present right now
    int alarm_locked;           // Flag if alarm is active (including safety cool-down)
    uint32_t fire_clear_tick;   // Timestamp tracking when the fire cleared
} SystemController_t;

// Modular Function Prototypes
void System_Init(SystemController_t *sys, float target, float tolerance);
float System_UpdateAverage(SystemController_t *sys, float new_temp);
void System_ProcessClimate(SystemController_t *sys, float avg_temp);
void System_ProcessFire(SystemController_t *sys);
void System_DisplayStatus(float avg_temp, SystemController_t *sys);
float Read_Temperature(void);
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SystemController_t mySystem;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t adc_val1 = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  System_Init(&mySystem, 26.0f, 20.0f);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  System_ProcessFire(&mySystem);
	  float current_temp = Read_Temperature();
	  float avg_temp = System_UpdateAverage(&mySystem, current_temp);
	  System_ProcessClimate(&mySystem, avg_temp);

	        // 3. Telemetry Output
	  System_DisplayStatus(avg_temp, &mySystem);

	        // 500ms cadence balances fast fire checking and clean terminal printing
	  HAL_Delay(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void System_Init(SystemController_t *sys, float target, float tolerance) {
    memset(sys->buffer, 0, sizeof(sys->buffer));
    sys->index = 0;
    sys->is_populated = 0;
    sys->target_temp = target;
    sys->tolerance_percent = tolerance;
    sys->state = SYSTEM_IDLE;
    sys->fire_detected = 0;
    sys->alarm_locked = 0;
    sys->fire_clear_tick = 0;
}

/**
  * @brief  Multiplexes and processes the fire sensor data alongside lock timers.
  */
void System_ProcessFire(SystemController_t *sys) {
    // Multiplexing check: sequentially read both inputs instantly
    GPIO_PinState s1 = HAL_GPIO_ReadPin(FIRE_SENSOR_1_GPIO_Port, FIRE_SENSOR_1_Pin);
    GPIO_PinState s2 = HAL_GPIO_ReadPin(FIRE_SENSOR_2_GPIO_Port, FIRE_SENSOR_2_Pin);

    // Active high sensor configuration logic
    if (s1 == GPIO_PIN_SET || s2 == GPIO_PIN_SET) {
        if (!sys->fire_detected) {
            sys->fire_detected = 1;
            sys->alarm_locked = 1;
        }
    } else {
        // Fire just stopped right now
        if (sys->fire_detected) {
            sys->fire_detected = 0;
            sys->fire_clear_tick = HAL_GetTick(); // Capture time of clearance
        }
    }

    // Lock Evaluation: If fire is clear, check if 5 seconds have elapsed
    if (!sys->fire_detected && sys->alarm_locked) {
        if ((HAL_GetTick() - sys->fire_clear_tick) >= ALARM_HOLD_TIME_MS) {
            sys->alarm_locked = 0; // Safe to unlock system
        }
    }

    // Actuate Safety Hardware
    if (sys->alarm_locked) {
        HAL_GPIO_WritePin(WATER_SPLIT_SYS_GPIO_Port, WATER_SPLIT_SYS_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(ALARM_BUZZER_GPIO_Port, ALARM_BUZZER_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(WATER_SPLIT_SYS_GPIO_Port, WATER_SPLIT_SYS_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ALARM_BUZZER_GPIO_Port, ALARM_BUZZER_Pin, GPIO_PIN_RESET);
    }
}

void System_ProcessClimate(SystemController_t *sys, float avg_temp) {
    // If the room is currently on fire, override normal fan/heater logic
    if (sys->alarm_locked) {
        HAL_GPIO_WritePin(BLUE_LED_FAN_GPIO_Port, BLUE_LED_FAN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RED_LED_HEATER_GPIO_Port, RED_LED_HEATER_Pin, GPIO_PIN_RESET);
        return;
    }

    float variance = sys->target_temp * (sys->tolerance_percent / 100.0f);
    float upper_threshold = sys->target_temp + variance;
    float lower_threshold = sys->target_temp - variance;

    if (avg_temp > upper_threshold) {
        sys->state = SYSTEM_COOLING;
        HAL_GPIO_WritePin(BLUE_LED_FAN_GPIO_Port, BLUE_LED_FAN_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(RED_LED_HEATER_GPIO_Port, RED_LED_HEATER_Pin, GPIO_PIN_RESET);
    }
    else if (avg_temp < lower_threshold) {
        sys->state = SYSTEM_HEATING;
        HAL_GPIO_WritePin(BLUE_LED_FAN_GPIO_Port, BLUE_LED_FAN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RED_LED_HEATER_GPIO_Port, RED_LED_HEATER_Pin, GPIO_PIN_SET);
    }
    else {
        sys->state = SYSTEM_IDLE;
        HAL_GPIO_WritePin(BLUE_LED_FAN_GPIO_Port, BLUE_LED_FAN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RED_LED_HEATER_GPIO_Port, RED_LED_HEATER_Pin, GPIO_PIN_RESET);
    }
}

float System_UpdateAverage(SystemController_t *sys, float new_temp) {
    sys->buffer[sys->index] = new_temp;
    sys->index = (sys->index + 1) % BUFFER_SIZE;
    if (sys->index == 0) sys->is_populated = 1;

    int items = sys->is_populated ? BUFFER_SIZE : sys->index;
    if (items == 0) return new_temp;

    float sum = 0.0f;
    for (int i = 0; i < items; i++) sum += sys->buffer[i];
    return sum / items;
}

void System_DisplayStatus(float avg_temp, SystemController_t *sys) {
    char terminal_msg[128];

    char safety_str[32];
    if (sys->fire_detected) {
        strcpy(safety_str, "CRITICAL FIRE!!");
    } else if (sys->alarm_locked) {
        uint32_t elapsed = HAL_GetTick() - sys->fire_clear_tick;
        long remaining = (ALARM_HOLD_TIME_MS - elapsed) / 1000;
        if (remaining < 0) remaining = 0;
        snprintf(safety_str, sizeof(safety_str), "LOCK-ON COOLING (%lds)", remaining);
    } else {
        strcpy(safety_str, "SECURE");
    }

    const char* climate_str = (sys->state == SYSTEM_COOLING) ? "COOLING" :
                              (sys->state == SYSTEM_HEATING) ? "HEATING" : "NORMAL";

    snprintf(terminal_msg, sizeof(terminal_msg),
             "Avg Temp: %.2f C | Climate: %s | Safety State: %s\r\n",
             avg_temp, climate_str, safety_str);

    HAL_UART_Transmit(&huart2, (uint8_t*)terminal_msg, strlen(terminal_msg), HAL_MAX_DELAY);
}

float Read_Temperature(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t raw_adc = HAL_ADC_GetValue(&hadc1);
    float voltage = (raw_adc * 3.3f) / 4096.0f;
    return (voltage - 0.5f) * 100.0f;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
