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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STATE_MAIN_MENU,
    STATE_WAIT_MAIN_CHOICE,
    
    STATE_WORKER_LOGIN,
    STATE_WAIT_WORKER_ID,
    STATE_WAIT_WORKER_MENU_ACTION,
    
    STATE_MANAGER_USER_PROMPT,
    STATE_MANAGER_WAIT_USER,
    STATE_MANAGER_PASS_PROMPT,
    STATE_MANAGER_WAIT_PASS,
    STATE_WAIT_MANAGER_MENU_ACTION
} SystemState;

SystemState currentState = STATE_MAIN_MENU;

char rx_buffer[50];
uint8_t rx_index = 0;
uint8_t rx_data;
volatile uint8_t input_ready = 0;

const char admin_user[] = "admin";
const char admin_pass[] = "1234";
char entered_user[20];
char entered_pass[20];

// ???? ???? ??? ???? ?????????? ???? Enter
void trim_string(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n')) {
        str[len-1] = '\0';
        len--;
    }
}
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
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
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    switch (currentState) {
        
        case STATE_MAIN_MENU:
            printf("\r\n\r\n=== SMART FACTORY CENTRAL TERMINAL ===\r\n");
            printf("1. Worker Login (Fingerprint ID)\r\n");
            printf("2. Foreman/Manager Login (Credentials)\r\n");
            printf("3. EMERGENCY (SOS)\r\n");
            printf("Select an option (1, 2, or 3): ");
            input_ready = 0;
            rx_index = 0;
            currentState = STATE_WAIT_MAIN_CHOICE;
            break;

        case STATE_WAIT_MAIN_CHOICE:
            if (input_ready) {
                input_ready = 0;
                trim_string(rx_buffer); 
                
                if (strcmp(rx_buffer, "1") == 0) {
                    currentState = STATE_WORKER_LOGIN;
                } else if (strcmp(rx_buffer, "2") == 0) {
                    currentState = STATE_MANAGER_USER_PROMPT;
                } else if (strcmp(rx_buffer, "3") == 0) {
                    printf("\r\n\r\n!!! EMERGENCY PROTOCOL INITIATED !!!\r\n");
                    printf("1. Fire Alarm\r\n2. Medical Emergency\r\n3. Hazardous Leak\r\n");
                    printf("Select Emergency Type: ");
                    currentState = STATE_MAIN_MENU; 
                } else {
                    printf("\r\nInvalid choice. Try again: ");
                    rx_index = 0;
                }
            }
            break;

        // ---------------- WORKER SECTION ----------------
        case STATE_WORKER_LOGIN:
            printf("\r\n\r\n[WORKER LOGIN] Please enter Fingerprint ID: ");
            input_ready = 0;
            rx_index = 0;
            currentState = STATE_WAIT_WORKER_ID;
            break;
            
        case STATE_WAIT_WORKER_ID:
            if (input_ready) {
                input_ready = 0;
                trim_string(rx_buffer);
                strcpy(entered_user, rx_buffer); 

                printf("\r\n========================================");
                printf("\r\n[ACCESS GRANTED] Welcome Worker ID: %s", rx_buffer);
                printf("\r\n========================================\r\n");
                printf("--- WORKER MENU ---\r\n");
                printf("1. Check-in\r\n2. Request Leave (Vacation)\r\n3. Send Letter\r\n4. Logout\r\n");
                printf("Select: ");
                
                currentState = STATE_WAIT_WORKER_MENU_ACTION;
            }
            break;

        case STATE_WAIT_WORKER_MENU_ACTION:
            if (input_ready) {
                input_ready = 0;
                trim_string(rx_buffer);
                
                if (rx_buffer[0] == '1') {
                    printf("CHECKIN:%s\r\n", entered_user);
                    printf("\r\n[SUCCESS] Check-in command sent to server.\r\n");
                } 
                else if (rx_buffer[0] == '2') {
                    // ????? ???? PA1 ???? ?????/?? ?????
                    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) {
                        printf("VACATION:%s:APPROVED\r\n", entered_user);
                        printf("\r\n[RESULT] Vacation APPROVED by Manager Panel.\r\n");
                    } else {
                        printf("VACATION:%s:REJECTED\r\n", entered_user);
                        printf("\r\n[RESULT] Vacation REJECTED by Manager Panel.\r\n");
                    }
                }
                else if (rx_buffer[0] == '3') {
                    // ????? ???? PA2 ???? ????? ????
                    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET) {
                        printf("LETTER:%s:READ\r\n", entered_user);
                        printf("\r\n[RESULT] Letter answered (Request file will be deleted).\r\n");
                    } else {
                        printf("LETTER:%s:PENDING\r\n", entered_user);
                        printf("\r\n[RESULT] Letter sent to queue (Pending).\r\n");
                    }
                }
                else if (rx_buffer[0] == '4') {
                    printf("\r\n[LOGGED OUT] Returning to Main Menu...\r\n");
                    currentState = STATE_MAIN_MENU;
                    break;
                }
                else {
                    printf("\r\nInvalid option.\r\n");
                }
                
                if (currentState == STATE_WAIT_WORKER_MENU_ACTION) {
                    printf("\r\nSelect next option (or 4 to logout): ");
                }
            }
            break;

        // ---------------- MANAGER SECTION ----------------
        case STATE_MANAGER_USER_PROMPT:
            printf("\r\n\r\n[MANAGER LOGIN]\r\nUsername: ");
            input_ready = 0;
            rx_index = 0;
            currentState = STATE_MANAGER_WAIT_USER;
            break;

        case STATE_MANAGER_WAIT_USER:
            if (input_ready) {
                input_ready = 0;
                trim_string(rx_buffer);
                strcpy(entered_user, rx_buffer);
                currentState = STATE_MANAGER_PASS_PROMPT;
            }
            break;

        case STATE_MANAGER_PASS_PROMPT:
            printf("\r\nPassword: ");
            input_ready = 0;
            rx_index = 0;
            currentState = STATE_MANAGER_WAIT_PASS;
            break;

        case STATE_MANAGER_WAIT_PASS:
            if (input_ready) {
                input_ready = 0;
                trim_string(rx_buffer);
                strcpy(entered_pass, rx_buffer);
                
                if (strcmp(entered_user, admin_user) == 0 && strcmp(entered_pass, admin_pass) == 0) {
                    printf("\r\n========================================");
                    printf("\r\n[ACCESS GRANTED] Welcome Foreman/Manager");
                    printf("\r\n========================================\r\n");
                    printf("--- MANAGER MENU ---\r\n");
                    printf("1. Stop Production Line\r\n2. Report Failure\r\n3. Send Demand/Letter\r\n4. Register Absence\r\n5. Logout\r\n");
                    printf("Select: ");
                    
                    currentState = STATE_WAIT_MANAGER_MENU_ACTION;
                } else {
                    printf("\r\n[ACCESS DENIED] Incorrect Username or Password.\r\n");
                    currentState = STATE_MAIN_MENU;
                }
            }
            break;

        case STATE_WAIT_MANAGER_MENU_ACTION:
            if (input_ready) {
                input_ready = 0;
                trim_string(rx_buffer);
                
                if (rx_buffer[0] == '1') {
                    printf("MANAGER:STOP_LINE\r\n");
                    printf("\r\n[ALARM] Production Line STOPPED command sent!\r\n");
                }
                else if (rx_buffer[0] == '2') {
                    printf("MANAGER:REPORT_FAIL\r\n");
                    printf("\r\n[SYSTEM] Failure report sent to database.\r\n");
                }
                else if (rx_buffer[0] == '3') {
                    printf("MANAGER:DEMAND\r\n");
                    printf("\r\n[SYSTEM] Demand letter sent to database.\r\n");
                }
                else if (rx_buffer[0] == '4') {
                    printf("MANAGER:ABSENCE\r\n");
                    printf("\r\n[SYSTEM] Absence record triggered.\r\n");
                }
                else if (rx_buffer[0] == '5') {
                    printf("\r\n[LOGGED OUT] Returning to Main Menu...\r\n");
                    currentState = STATE_MAIN_MENU;
                    break;
                }
                else {
                    printf("\r\nInvalid option.\r\n");
                }
                
                if (currentState == STATE_WAIT_MANAGER_MENU_ACTION) {
                    printf("\r\nSelect next option (or 5 to logout): ");
                }
            }
            break;
    }
    HAL_Delay(50);
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

  /*Configure GPIO pin : FINGERPRINT_SENSOR_Pin */
  GPIO_InitStruct.Pin = FINGERPRINT_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FINGERPRINT_SENSOR_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        if (rx_data == '\r' || rx_data == '\n') {
            rx_buffer[rx_index] = '\0';
            input_ready = 1;            
        } else {
            if (rx_index < sizeof(rx_buffer) - 1) {
                rx_buffer[rx_index++] = rx_data;
                HAL_UART_Transmit(&huart1, &rx_data, 1, 10);
            }
        }
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
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
