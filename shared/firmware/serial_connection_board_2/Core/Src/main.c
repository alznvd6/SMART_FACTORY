/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for Board 2
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

uint32_t lastDisplayTick = 0;

uint8_t rxByte;
char rxBuffer[50];
uint8_t rxIndex = 0;
volatile uint8_t lineReadyFromTerminal = 0;
char messageFromBoard1[50] = "None";
char sharedAdcValue[20] = "0";

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void printLocalConsole(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  // Explicitly enable USART1 Global Interrupt line
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  // Clear Terminal 2
  char clearScreen[] = "\033[2J\033[H";
  HAL_UART_Transmit(&huart1, (uint8_t*)clearScreen, strlen(clearScreen), HAL_MAX_DELAY);

  HAL_UART_Receive_IT(&huart1, &rxByte, 1);

  while (1)
  {
      if (HAL_GetTick() - lastDisplayTick >= 500)
      {
          lastDisplayTick = HAL_GetTick();
          printLocalConsole();
      }

      if (lineReadyFromTerminal == 1)
      {
          char txPacket[65];
          snprintf(txPacket, sizeof(txPacket), "#%s\n", rxBuffer);
          HAL_UART_Transmit(&huart1, (uint8_t*)txPacket, strlen(txPacket), 100);

          memset(rxBuffer, 0, sizeof(rxBuffer));
          rxIndex = 0;
          lineReadyFromTerminal = 0;

          printLocalConsole();
          HAL_UART_Receive_IT(&huart1, &rxByte, 1);
      }
  }
}

//void printLocalConsole(void)
//{
//    char homeCursor[] = "\033[H";
//    HAL_UART_Transmit(&huart1, (uint8_t*)homeCursor, strlen(homeCursor), 100);
//
//    char uiBuffer[250];
//    snprintf(uiBuffer, sizeof(uiBuffer),
//             "==========================================\r\n"
//             "               BOARD #2 CONSOLE           \r\n"
//             "  Remote ADC Value from U1: %-4s         \r\n"
//             "==========================================\r\n"
//             "  Latest Msg From Board #1: %-20s       \r\n"
//             "==========================================\r\n"
//             "  Your msg for board #1 (Type + Enter):\r\n"
//             "  > %-35s",
//             sharedAdcValue, messageFromBoard1, rxBuffer);
//
//    HAL_UART_Transmit(&huart1, (uint8_t*)uiBuffer, strlen(uiBuffer), 100);
//}
void printLocalConsole(void)
{
    // حذف کدهای اسکی کنترلی برای جلوگیری از قاطی کردن پروتئوس
    char uiBuffer[200];
    snprintf(uiBuffer, sizeof(uiBuffer),
             "\r\n--- BOARD #2 ---\r\n"
             "Remote ADC: %s\r\n"
             "From Board 1: %s\r\n"
             "Your msg: > %s\r\n",
             sharedAdcValue, messageFromBoard1, rxBuffer);

    HAL_UART_Transmit(&huart1, (uint8_t*)uiBuffer, strlen(uiBuffer), 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (rxByte == '\r' || rxByte == '\n')
        {
            if (rxIndex > 0)
            {
                rxBuffer[rxIndex] = '\0';

                if (rxBuffer[0] == '@')
                {
                    strncpy(messageFromBoard1, &rxBuffer[1], sizeof(messageFromBoard1) - 1);
                    rxIndex = 0;
                    memset(rxBuffer, 0, sizeof(rxBuffer));
                    printLocalConsole();
                }
                else if (strncmp(rxBuffer, "ADC=", 4) == 0)
                {
                    strncpy(sharedAdcValue, &rxBuffer[4], sizeof(sharedAdcValue) - 1);
                    rxIndex = 0;
                    memset(rxBuffer, 0, sizeof(rxBuffer));
                    printLocalConsole();
                }
                else
                {
                    lineReadyFromTerminal = 1;
                    return;
                }
            }
        }
        else if (rxIndex < sizeof(rxBuffer) - 1)
        {
            HAL_UART_Transmit(&huart1, &rxByte, 1, 10);
            rxBuffer[rxIndex++] = (char)rxByte;
        }
        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}

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
  HAL_UART_Init(&huart1);
}

static void MX_GPIO_Init(void) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
void Error_Handler(void) { __disable_irq(); while (1); }
