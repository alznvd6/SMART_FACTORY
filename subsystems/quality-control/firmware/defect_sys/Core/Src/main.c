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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_RS_GPIO_Port   GPIOC
#define LCD_RS_Pin         GPIO_PIN_13
#define LCD_EN_GPIO_Port   GPIOC
#define LCD_EN_Pin         GPIO_PIN_14

#define LCD_D4_GPIO_Port   GPIOC
#define LCD_D4_Pin         GPIO_PIN_15
#define LCD_D5_GPIO_Port   GPIOA
#define LCD_D5_Pin         GPIO_PIN_8
#define LCD_D6_GPIO_Port   GPIOA
#define LCD_D6_Pin         GPIO_PIN_9
#define LCD_D7_GPIO_Port   GPIOA
#define LCD_D7_Pin         GPIO_PIN_10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int total_products = 0;
int healthy_products = 0;
int defect_products = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_SendString(char *str);
void LCD_SetCursor(uint8_t row, uint8_t col);
void Update_LCD_Display(int total, int healthy, int defect);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();

    // Display Startup Splash Screen
  LCD_SetCursor(1, 1);
  LCD_SendString("System Ready...");
  HAL_Delay(1000);
  LCD_Command(0x01); // Flush and Clear LCD

    // Print initial dashboard zeros
  Update_LCD_Display(total_products, healthy_products, defect_products);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
      // Check if the primary detection IR beam goes Active High (Product Enters)
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
      {
        total_products++;

        // Instantly poll the validation sensor state
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET)
        {
          // Defect identified
          defect_products++;

          // Assert Red Status LED
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

          // Transmit serial log (Sent via remapped PB6 TX pin)
          char *msg = "problem\r\n";
          HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        }
        else
        {
          // Healthy unit confirmed
          healthy_products++;

          // Assert Green Status LED
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

          // Transmit serial log
          char *msg = "healthy\r\n";
          HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        }

        // Redraw updating data telemetry directly on the display
        Update_LCD_Display(total_products, healthy_products, defect_products);

        // Hold state for the exact 2-second processing pipeline window
        HAL_Delay(2000);

        // Clear indicator LEDs to clear the runway for the next physical unit
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
      }

      HAL_Delay(50); // Small cycle delay to prevent simulation processing locking
      /* USER CODE END WHILE */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}
/* USER CODE BEGIN 4 */
void LCD_EnablePulse(void)
{
  HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
}

void LCD_Send4Bit(uint8_t val)
{
  // Separately paths each out to their explicit pin configurations
  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (val >> 0) & 0x01);
  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (val >> 1) & 0x01);
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (val >> 2) & 0x01);
  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (val >> 3) & 0x01);
  LCD_EnablePulse();
}

void LCD_Command(uint8_t cmd)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  LCD_Send4Bit(cmd >> 4);   // Transmission Phase 1: High Nibble
  LCD_Send4Bit(cmd & 0x0F);  // Transmission Phase 2: Low Nibble
}

void LCD_Data(uint8_t data)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
  LCD_Send4Bit(data >> 4);
  LCD_Send4Bit(data & 0x0F);
}

void LCD_Init(void)
{
  HAL_Delay(50);
  LCD_Send4Bit(0x03);
  HAL_Delay(5);
  LCD_Send4Bit(0x03);
  HAL_Delay(1);
  LCD_Send4Bit(0x03);
  LCD_Send4Bit(0x02); // Enforce 4-Bit Operating Topology

  LCD_Command(0x28); // 2 Display Rows, standard 5x7 Dot matrix character array
  LCD_Command(0x0C); // Display Activated, Blinking tracking cursor suppressed
  LCD_Command(0x06); // Set text direction to automatically increment right
  LCD_Command(0x01); // Flush/Wipe memory clean
  HAL_Delay(2);
}

void LCD_SendString(char *str)
{
  while(*str) {
    LCD_Data(*str++);
  }
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
  uint8_t address = (row == 1) ? (0x80 + col - 1) : (0xC0 + col - 1);
  LCD_Command(address);
}

void Update_LCD_Display(int total, int healthy, int defect)
{
  char line1[17];
  char line2[17];

  // Dynamically pack integer counters inside custom 16-character array lines
  sprintf(line1, "Total: %-10d", total);
  sprintf(line2, "H:%-4d | D:%-4d", healthy, defect);

  LCD_SetCursor(1, 1);
  LCD_SendString(line1);
  LCD_SetCursor(2, 1);
  LCD_SendString(line2);
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
