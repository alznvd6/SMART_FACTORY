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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define HISTORY_SIZE 20
#define LCD_COLS     20

typedef enum
{
  LCD_STATE_OFF = 0,
  LCD_STATE_WAIT,
  LCD_STATE_REPORT
}LCD_StateTypeDef;

typedef struct
{
  uint8_t valid;
  uint8_t status;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
}ProductRecord;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PRODUCT_PASS    1
#define PRODUCT_REJECT  0
#define WEIGHT_MIN_ADC   100
#define WEIGHT_MAX_ADC   3000
#define LENGTH_MIN_CM    5
#define LENGTH_MAX_CM    12
#define COLOR_OK_STATE   GPIO_PIN_SET
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t system_on = 0;
uint8_t motion_flag = 0;
uint8_t product_busy = 0;

uint32_t total_count = 0;
uint32_t pass_count = 0;
uint32_t reject_count = 0;

uint8_t last_product_valid = 0;
uint8_t last_product_status = PRODUCT_REJECT;

uint16_t adc_value = 0;
uint8_t color_value = 0;

uint8_t relay_busy = 0;
uint8_t relay_phase = 0;
uint32_t relay_tick = 0;

uint8_t report_active = 0;
uint32_t report_tick = 0;

LCD_StateTypeDef lcd_state = LCD_STATE_OFF;

ProductRecord history[HISTORY_SIZE];
uint8_t history_index = 0;

uint16_t length_value = 0;
uint8_t ultra_rx_buf[8];

char lcd_text[21];
char uart_text[160];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

void Update_Ultrasonic_Data(void);
/* USER CODE BEGIN PFP */
void LCD_Send4Bit(uint8_t data);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Init_20x4(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_SendString(char *str);
void LCD_PrintLine(uint8_t row, char *text);

void System_ResetAllData(void);
void Get_SystemTime(uint8_t *hour, uint8_t *minute, uint8_t *second);
void Save_History(uint8_t status);
uint8_t Check_ProductStatus(void);
void Process_Product(void);
void Update_LCD(void);
void Update_Relay(void);
void Send_UART_Report(void);
uint16_t Read_Ultrasonic_DistanceCm(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_Send4Bit(uint8_t data)
{
  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (GPIO_PinState)((data >> 0) & 0x01));
  HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (GPIO_PinState)((data >> 1) & 0x01));
  HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (GPIO_PinState)((data >> 2) & 0x01));
  HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (GPIO_PinState)((data >> 3) & 0x01));

  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
}

void LCD_SendCommand(uint8_t cmd)
{
  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
  LCD_Send4Bit((cmd >> 4) & 0x0F);
  LCD_Send4Bit(cmd & 0x0F);
}

void LCD_SendData(uint8_t data)
{
  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET);
  LCD_Send4Bit((data >> 4) & 0x0F);
  LCD_Send4Bit(data & 0x0F);
}

void LCD_Init_20x4(void)
{
  HAL_Delay(40);

  HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);

  LCD_Send4Bit(0x03);
  HAL_Delay(5);
  LCD_Send4Bit(0x03);
  HAL_Delay(1);
  LCD_Send4Bit(0x03);
  HAL_Delay(1);
  LCD_Send4Bit(0x02);

  LCD_SendCommand(0x28);
  LCD_SendCommand(0x0C);
  LCD_SendCommand(0x06); 
  LCD_SendCommand(0x01); 
  HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
  uint8_t address = 0;

  switch(row)
  {
    case 0: address = 0x00 + col; break;
    case 1: address = 0x40 + col; break;
    case 2: address = 0x14 + col; break;
    case 3: address = 0x54 + col; break;
    default: address = 0x00 + col; break;
  }

  LCD_SendCommand(0x80 | address);
}

void LCD_SendString(char *str)
{
  while(*str)
  {
    LCD_SendData((uint8_t)*str);
    str++;
  }
}

void LCD_PrintLine(uint8_t row, char *text)
{
  uint8_t i;
  LCD_SetCursor(row, 0);

  for(i = 0; i < LCD_COLS; i++)
  {
    if(text[i] != '\0')
      LCD_SendData(text[i]);
    else
      LCD_SendData(' ');
  }
}

void System_ResetAllData(void)
{
  uint8_t i;

  total_count = 0;
  pass_count = 0;
  reject_count = 0;

  last_product_valid = 0;
  last_product_status = PRODUCT_REJECT;

  motion_flag = 0;
  product_busy = 0;

  adc_value = 0;
  color_value = 0;

  relay_busy = 0;
  relay_phase = 0;
  relay_tick = 0;

  report_active = 0;
  report_tick = 0;

  history_index = 0;
  for(i = 0; i < HISTORY_SIZE; i++)
  {
    history[i].valid = 0;
    history[i].status = PRODUCT_REJECT;
    history[i].hour = 0;
    history[i].minute = 0;
    history[i].second = 0;
  }

  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RELAY_FWD_GPIO_Port, RELAY_FWD_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RELAY_REV_GPIO_Port, RELAY_REV_Pin, GPIO_PIN_RESET);
}

void Get_SystemTime(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
  uint32_t total_seconds = HAL_GetTick() / 1000;
  *hour   = (total_seconds / 3600) % 24;
  *minute = (total_seconds / 60) % 60;
  *second = total_seconds % 60;
}

void Save_History(uint8_t status)
{
  uint8_t hour, minute, second;

  Get_SystemTime(&hour, &minute, &second);

  history[history_index].valid = 1;
  history[history_index].status = status;
  history[history_index].hour = hour;
  history[history_index].minute = minute;
  history[history_index].second = second;

  history_index++;
  if(history_index >= HISTORY_SIZE)
    history_index = 0;
}

uint8_t Check_ProductStatus(void)
{
  uint8_t weight_ok;
  uint8_t color_ok;
	uint8_t length_ok = 0;

	
	Update_Ultrasonic_Data();
  HAL_ADC_Start(&hadc1);
	HAL_Delay(2);
  HAL_ADC_PollForConversion(&hadc1, 20);
  adc_value = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);


	color_value = (uint8_t)HAL_GPIO_ReadPin(COLOR_GPIO_Port, COLOR_Pin);

  

  if((adc_value >= WEIGHT_MIN_ADC) && (adc_value <= WEIGHT_MAX_ADC))
    weight_ok = 1;
  else
    weight_ok = 0;

  if(color_value == COLOR_OK_STATE)
    color_ok = 1;
  else
    color_ok = 0;
	
	if((length_value >= LENGTH_MIN_CM) && (length_value <= LENGTH_MAX_CM))
		length_ok = 1;
	else
		length_ok = 0;

  if((weight_ok == 1))
    return PRODUCT_PASS;
  else
    return PRODUCT_REJECT;
}


void Send_UART_Report(void)
{
  uint16_t len;
  uint8_t i;
  uint8_t index;

  len = (uint16_t)sprintf(uart_text, "TOTAL=%lu,PASS=%lu,REJECT=%lu\r\n",
                          total_count, pass_count, reject_count);
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_text, len, 200);

  len = (uint16_t)sprintf(uart_text, "LAST20:\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_text, len, 200);

  for(i = 0; i < HISTORY_SIZE; i++)
  {
    index = (history_index + i) % HISTORY_SIZE;

    if(history[index].valid == 1)
    {
      len = (uint16_t)sprintf(uart_text, "%02d) %s %02d:%02d:%02d\r\n",
                              i + 1,
                              (history[index].status == PRODUCT_PASS) ? "PASS" : "REJECT",
                              history[index].hour,
                              history[index].minute,
                              history[index].second);
      HAL_UART_Transmit(&huart2, (uint8_t*)uart_text, len, 200);
    }
  }
}

void Process_Product(void)
{
  uint8_t status;

  motion_flag = 0;
  product_busy = 1;

  status = Check_ProductStatus();
  total_count++;
  last_product_valid = 1;
  last_product_status = status;

  if(status == PRODUCT_PASS)
  {
    pass_count++;
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(RELAY_FWD_GPIO_Port, RELAY_FWD_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RELAY_REV_GPIO_Port, RELAY_REV_Pin, GPIO_PIN_RESET);
  }
  else
  {
    reject_count++;
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(RELAY_FWD_GPIO_Port, RELAY_FWD_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RELAY_REV_GPIO_Port, RELAY_REV_Pin, GPIO_PIN_RESET);

    relay_busy = 1;
    relay_phase = 1;
    relay_tick = HAL_GetTick();
  }

  Save_History(status);
  sprintf(uart_text, "ADC=%u COLOR=%u LENGTH=%u STATUS=%s\r\n",
        adc_value,
        color_value,
				length_value,
        (status == PRODUCT_PASS) ? "PASS" : "REJECT");
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_text, strlen(uart_text), 200);

  Send_UART_Report();

  report_active = 1;
  report_tick = HAL_GetTick();
  lcd_state = LCD_STATE_REPORT;

  product_busy = 0;
}

void Update_Relay(void)
{
  if(relay_busy == 1)
  {
    if(relay_phase == 1)
    {
      if((HAL_GetTick() - relay_tick) >= 500)
      {
        HAL_GPIO_WritePin(RELAY_FWD_GPIO_Port, RELAY_FWD_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RELAY_REV_GPIO_Port, RELAY_REV_Pin, GPIO_PIN_SET);

        relay_phase = 2;
        relay_tick = HAL_GetTick();
      }
    }
    else if(relay_phase == 2)
    {
      if((HAL_GetTick() - relay_tick) >= 800)
      {
        HAL_GPIO_WritePin(RELAY_FWD_GPIO_Port, RELAY_FWD_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RELAY_REV_GPIO_Port, RELAY_REV_Pin, GPIO_PIN_RESET);

        relay_phase = 0;
        relay_busy = 0;
      }
    }
  }
}

void Update_LCD(void)
{
  uint8_t hour, minute, second;

  Get_SystemTime(&hour, &minute, &second);

  switch(lcd_state)
  {
    case LCD_STATE_OFF:
      LCD_PrintLine(0, "TIME 00:00:00       ");
      LCD_PrintLine(1, "TOTAL: 0            ");
      LCD_PrintLine(2, "PASS:0 REJECT:0     ");
      LCD_PrintLine(3, "SYSTEM OFF          ");
      break;

    case LCD_STATE_WAIT:
      sprintf(lcd_text, "TIME %02d:%02d:%02d       ", hour, minute, second);
      LCD_PrintLine(0, lcd_text);

      sprintf(lcd_text, "TOTAL: %lu          ", total_count);
      LCD_PrintLine(1, lcd_text);

      sprintf(lcd_text, "PASS:%lu REJECT:%lu", pass_count, reject_count);
      LCD_PrintLine(2, lcd_text);

      LCD_PrintLine(3, "WAITING PRODUCT     ");
      break;

    case LCD_STATE_REPORT:
      sprintf(lcd_text, "TIME %02d:%02d:%02d       ", hour, minute, second);
      LCD_PrintLine(0, lcd_text);

      sprintf(lcd_text, "TOTAL: %lu          ", total_count);
      LCD_PrintLine(1, lcd_text);

      sprintf(lcd_text, "PASS:%lu REJECT:%lu", pass_count, reject_count);
      LCD_PrintLine(2, lcd_text);
			
			if((HAL_GetTick() - report_tick) >= 2000)
			{
				report_active = 0;
				lcd_state = LCD_STATE_WAIT;

				HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
			}


      if(last_product_valid == 1)
      {
        if(last_product_status == PRODUCT_PASS)
          LCD_PrintLine(3, "LAST: PASS          ");
        else
          LCD_PrintLine(3, "LAST: REJECT        ");
      }
      else
      {
        LCD_PrintLine(3, "NO PRODUCT          ");
      }

      if((HAL_GetTick() - report_tick) >= 2000)
      {
        report_active = 0;
        lcd_state = LCD_STATE_WAIT;
      }
      break;

    default:
      lcd_state = LCD_STATE_OFF;
      break;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == ON_OFF_Pin)
  {
    if(system_on == 0)
    {
      system_on = 1;
      System_ResetAllData();
      lcd_state = LCD_STATE_WAIT;
    }
    else
    {
      system_on = 0;
      System_ResetAllData();
      lcd_state = LCD_STATE_OFF;
    }
  }

  if(GPIO_Pin == MOTION_Pin)
  {
    if((system_on == 1) && (product_busy == 0))
      motion_flag = 1;
  }
}
void Update_Ultrasonic_Data(void)
{
  uint8_t cmd = 0x55;
  
  HAL_UART_Transmit(&huart1, &cmd, 1, 50);
  
  if(HAL_UART_Receive(&huart1, ultra_rx_buf, 2, 100) == HAL_OK)
  {
    length_value = (ultra_rx_buf[0] << 8) | ultra_rx_buf[1];
  }
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  LCD_Init_20x4();
  System_ResetAllData();
  lcd_state = LCD_STATE_OFF;
  Update_LCD();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

		if(system_on == 0)
		{
			lcd_state = LCD_STATE_OFF;
		}
		else
		{
			if((motion_flag == 1) && (product_busy == 0))
			{
				Process_Product();
			}

			if((report_active == 0) && (lcd_state != LCD_STATE_REPORT))
			{
				lcd_state = LCD_STATE_WAIT;
			}
		}

		Update_Relay();
		Update_LCD();

		HAL_Delay(100);


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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
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

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RELAY_REV_Pin|RELAY_FWD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_GREEN_Pin|LED_RED_Pin|D7_Pin|RS_Pin
                          |E_Pin|D4_Pin|D5_Pin|D6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : COLOR_Pin */
  GPIO_InitStruct.Pin = COLOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(COLOR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MOTION_Pin */
  GPIO_InitStruct.Pin = MOTION_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MOTION_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ON_OFF_Pin */
  GPIO_InitStruct.Pin = ON_OFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ON_OFF_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RELAY_REV_Pin RELAY_FWD_Pin */
  GPIO_InitStruct.Pin = RELAY_REV_Pin|RELAY_FWD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GREEN_Pin LED_RED_Pin D7_Pin RS_Pin
                           E_Pin D4_Pin D5_Pin D6_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_RED_Pin|D7_Pin|RS_Pin
                          |E_Pin|D4_Pin|D5_Pin|D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
