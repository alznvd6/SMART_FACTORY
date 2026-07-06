/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
// Quality Control Counters
uint32_t total_passed = 0;
uint32_t total_failed = 0;

// Interrupt Flag (Set to 1 when PB0 goes High)
volatile uint8_t item_detected = 0; 

// Sensor Variables
uint16_t weight_value = 0;
uint32_t distance_cm = 0;
uint8_t color_code = 0;

// System Clock / Time simulation (Daily Report at 07:00 AM)
uint8_t current_hour = 6;     // Start at 06:00 AM
uint8_t current_minute = 58;  // 58 minutes (flows to 07:00 AM soon)
uint32_t time_tick = 0;       

// Serial communication buffer
char uart_buffer[128];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Custom Printf function for clean, line-by-line output in Proteus Terminal
void uart_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(uart_buffer, sizeof(uart_buffer), fmt, args);
    va_end(args);

    // Send formatted string
    HAL_UART_Transmit(&huart1, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
    // Send Carriage Return and Line Feed for newline
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);
}

// External Interrupt Callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ITEM_SENSOR_Pin)
    {
        item_detected = 1; // Trigger flag when button/sensor on PB0 is pressed
    }
}

// Read Weight from Analog ADC (PA0)
uint16_t read_weight(void)
{
    uint16_t adc_val = 0;
    HAL_ADC_Start(&hadc1);
    
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        adc_val = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return adc_val;
}

// Read Distance from Ultrasonic Sensor (PA1: Trig, PA2: Echo)
uint32_t read_distance(void)
{
    uint32_t pulse_time = 0;
    uint32_t timeout = 10000; // Timeout to prevent Proteus simulator lockup
    
    // Generate 10 microsecond pulse on Trig
    HAL_GPIO_WritePin(US_TRIG_GPIO_Port, US_TRIG_Pin, GPIO_PIN_SET);
    HAL_Delay(1); 
    HAL_GPIO_WritePin(US_TRIG_GPIO_Port, US_TRIG_Pin, GPIO_PIN_RESET);
    
    // Wait for Echo pin to go HIGH
    while(HAL_GPIO_ReadPin(US_ECHO_GPIO_Port, US_ECHO_Pin) == GPIO_PIN_RESET && timeout > 0)
    {
        timeout--;
    }
    
    timeout = 10000;
    // Measure pulse width
    while(HAL_GPIO_ReadPin(US_ECHO_GPIO_Port, US_ECHO_Pin) == GPIO_PIN_SET && timeout > 0)
    {
        pulse_time++;
        timeout--;
        HAL_Delay(1); 
    }
    
    return (pulse_time * 2); // Simplified distance calculation for simulation
}

// Read Color Code from PB1
uint8_t read_color(void)
{
    // Reads GPIOB Pin 1 (1 = Correct color, 0 = Wrong color)
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
    {
        return 1; 
    }
    return 0;
}

// Report local status to Main Board
void report_to_main(uint8_t status)
{
    if (status == 1) // PASS
    {
        HAL_GPIO_WritePin(MAIN_OK_GPIO_Port, MAIN_OK_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MAIN_FAIL_GPIO_Port, MAIN_FAIL_Pin, GPIO_PIN_RESET);
    }
    else // FAIL
    {
        HAL_GPIO_WritePin(MAIN_OK_GPIO_Port, MAIN_OK_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MAIN_FAIL_GPIO_Port, MAIN_FAIL_Pin, GPIO_PIN_SET);
    }
    
    // Ready signal active high
    HAL_GPIO_WritePin(MAIN_READY_GPIO_Port, MAIN_READY_Pin, GPIO_PIN_SET);
    HAL_Delay(100); 
}

// Clear all report lines
void reset_main_report(void)
{
    HAL_GPIO_WritePin(MAIN_OK_GPIO_Port, MAIN_OK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MAIN_FAIL_GPIO_Port, MAIN_FAIL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MAIN_READY_GPIO_Port, MAIN_READY_Pin, GPIO_PIN_RESET);
}
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
  MX_USART1_UART_Init();
  
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
  reset_main_report();
  
  // Clean startup messages
  uart_printf("=== QC System Started ===");
  uart_printf("Waiting for items...");
  uart_printf("---------------------------------");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 1. Process item when interrupt occurs on PB0
    if (item_detected == 1)
    {
        weight_value = read_weight();
        distance_cm = read_distance();
        color_code = read_color();
        
        // Quality Criteria check:
        // - Weight: between 1500 and 3500
        // - Distance (size): less than 25 cm
        // - Color: Correct (1)
        if (weight_value >= 1500 && weight_value <= 3500 && distance_cm < 25 && color_code == 1)
        {
            total_passed++;
            
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
            
            uart_printf("Item status: PASS | Weight: %d | Size: %d cm | Color: OK | Total OK: %d", 
                        weight_value, (int)distance_cm, total_passed);
            
            report_to_main(1);
        }
        else
        {
            total_failed++;
            
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
            
            uart_printf("Item status: FAIL | Weight: %d | Size: %d cm | Color: %s | Total FAIL: %d", 
                        weight_value, (int)distance_cm, (color_code == 1 ? "OK" : "WRONG"), total_failed);
            
            report_to_main(2);
        }
        
        HAL_Delay(2000); // Wait 2 seconds to hold output
        
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
        reset_main_report();
        
        item_detected = 0; // Clear trigger flag
    }
    
    // 2. Increment System Time
    time_tick++;
    if (time_tick >= 10) 
    {
        time_tick = 0;
        current_minute++;
        if (current_minute >= 60)
        {
            current_minute = 0;
            current_hour++;
            if (current_hour >= 24)
            {
                current_hour = 0;
            }
        }
    }
    
    // 3. Daily Report generation at 07:00 AM
    if (current_hour == 7 && current_minute == 0)
    {
        uart_printf("=================================");
        uart_printf("    DAILY QC REPORT - 07:00 AM   ");
        uart_printf("=================================");
        uart_printf("Total Passed (OK)    : %d", (int)total_passed);
        uart_printf("Total Failed (Error) : %d", (int)total_failed);
        uart_printf("---------------------------------\r\n");
        
        total_passed = 0;
        total_failed = 0;
        
        HAL_Delay(1200); // Simple delay to prevent re-triggering within the same minute
    }
    
    HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(US_TRIG_GPIO_Port, US_TRIG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_RED_Pin|MAIN_OK_Pin|MAIN_FAIL_Pin
                          |MAIN_READY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : US_TRIG_Pin */
  GPIO_InitStruct.Pin = US_TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(US_TRIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : US_ECHO_Pin */
  GPIO_InitStruct.Pin = US_ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(US_ECHO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ITEM_SENSOR_Pin */
  GPIO_InitStruct.Pin = ITEM_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ITEM_SENSOR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 (COLOR_A_Pin) */
  GPIO_InitStruct.Pin = COLOR_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GREEN_Pin LED_RED_Pin MAIN_OK_Pin MAIN_FAIL_Pin
                           MAIN_READY_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_RED_Pin|MAIN_OK_Pin|MAIN_FAIL_Pin
                          |MAIN_READY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
