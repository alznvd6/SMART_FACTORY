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
#include <stdbool.h>
#include <stdlib.h>
#include "lcd.h"
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
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
bool lcd_needs_update = false; 

char current_error_id[20] = {0};
char current_error_action[30] = {0};

typedef struct {
    char name[50];
    int phase;
    char target[20];
    bool is_valid;
} TaskDisplay;

TaskDisplay lcd_queue[2]; 

uint8_t rx_data1, rx_data2;
char buff1[64], buff2[128];
int idx1 = 0, idx2 = 0;

bool is_error_active = false;
bool is_error_solved_screen = false;
uint32_t error_solved_timer = 0;

bool is_worker_screen_active = false;
int worker_screen_line = 0; 
uint32_t worker_screen_timer = 0;
char worker_id_to_show[20] = {0};

void Process_Terminal_Command(char* cmd);
void Process_Central_Command(char* cmd);
void Refresh_LCD(void);
void Shift_Queue_Up(int line_to_clear);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
bool Warning( char buff[30] );
void Transfer( char buff[30] );

void Check_Central_Commands(char* cmd_buff);
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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1, &rx_data1, 1);
	HAL_UART_Receive_IT(&huart2, &rx_data2, 1);

	LCD_Init();
	lcd_queue[0].is_valid = false;
	lcd_queue[1].is_valid = false;

	lcd_needs_update = true;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (lcd_needs_update) {
          Refresh_LCD();
          lcd_needs_update = false; 
      }

      if (is_error_solved_screen) {
          if (HAL_GetTick() - error_solved_timer >= 1000) {
              is_error_solved_screen = false;
              HAL_UART_Transmit(&huart2, (uint8_t*)"CMD_TIMER:RESUME\r\n", 18, 100);
              lcd_needs_update = true; 
          }
      }

      if (is_worker_screen_active && !is_error_active && !is_error_solved_screen) {
          if (HAL_GetTick() - worker_screen_timer >= 5000) {
              is_worker_screen_active = false;
              Shift_Queue_Up(worker_screen_line);
              lcd_needs_update = true; 
          }
      }

    /* USER CODE END WHILE */
  }

    /* USER CODE BEGIN 3 */
}
  /* USER CODE END 3 */


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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RS_Pin|E_Pin|D4_Pin|D5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, D6_Pin|D7_Pin|LED_Pin|BUZ_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RS_Pin E_Pin D4_Pin D5_Pin */
  GPIO_InitStruct.Pin = RS_Pin|E_Pin|D4_Pin|D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : D6_Pin D7_Pin LED_Pin BUZ_Pin */
  GPIO_InitStruct.Pin = D6_Pin|D7_Pin|LED_Pin|BUZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        if (rx_data1 == '\n' || rx_data1 == '\r') {
            if (idx1 > 0) {
                buff1[idx1] = '\0';
                Process_Terminal_Command(buff1);
                idx1 = 0;
            }
        } else {
            if (idx1 < sizeof(buff1) - 1) {
                buff1[idx1++] = rx_data1;
            }
        }
        HAL_UART_Receive_IT(&huart1, &rx_data1, 1);
    }

    if (huart->Instance == USART2) {
        if (rx_data2 == '\n' || rx_data2 == '\r') {
            if (idx2 > 0) {
                buff2[idx2] = '\0';
                Process_Central_Command(buff2);
                idx2 = 0;
            }
        } else {
            if (idx2 < sizeof(buff2) - 1) {
                buff2[idx2++] = rx_data2;
            }
        }
        HAL_UART_Receive_IT(&huart2, &rx_data2, 1);
    }
}

void Process_Terminal_Command(char* cmd) {
    char id[20] = {0};
    char action[30] = {0};

    // ???? ???? ?? ???? (:) ?? ???? ?????
    char* colon_ptr = strchr(cmd, ':');
    if (colon_ptr) {
        int id_len = colon_ptr - cmd;
        if (id_len < 20) {
            strncpy(id, cmd, id_len);
            id[id_len] = '\0';
        }
        
        // ??????? Action (??? ????????? ??????? ??? ?? ?? ????)
        char* action_ptr = colon_ptr + 1;
        while(*action_ptr == ' ') action_ptr++; 
        strncpy(action, action_ptr, sizeof(action)-1);

        // ??? ?????????? ????? ?? ?????? ????
        action[strcspn(action, "\r\n")] = '\0';

        if (strcmp(action, "CLOTH") == 0 || strcmp(action, "DISTANCE") == 0) {
            is_error_active = true;
            is_worker_screen_active = false;
            
            strcpy(current_error_id, id);
            strcpy(current_error_action, action);
            
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
            
            HAL_UART_Transmit(&huart2, (uint8_t*)"CMD_TIMER:FREEZE\r\n", 18, 100);
            
            char log_pkg[128];
            snprintf(log_pkg, sizeof(log_pkg), "LOG_ERROR:ID:%s:%s\r\n", id, action);
            HAL_UART_Transmit(&huart2, (uint8_t*)log_pkg, strlen(log_pkg), 100);
            
            lcd_needs_update = true; 
        }
        else if (strcmp(action, "-") == 0) {
            if (is_error_active) {
                is_error_active = false;
                is_error_solved_screen = true;
                
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
                
                error_solved_timer = HAL_GetTick();
                lcd_needs_update = true;
            }
        }
        else if (strcmp(action, "1") == 0 || strcmp(action, "2") == 0) {
            if (is_error_active || is_error_solved_screen || is_worker_screen_active) return;

            int selected_line = atoi(action) - 1;

            if (lcd_queue[selected_line].is_valid) {
                char tx_central[128];
                snprintf(tx_central, sizeof(tx_central), "%s: BONUS\r\n", id);
                HAL_UART_Transmit(&huart2, (uint8_t*)tx_central, strlen(tx_central), 100);
                
                strcpy(worker_id_to_show, id);
                worker_screen_line = selected_line;
            }
        }
    }
}

void Process_Central_Command(char* cmd) {
    if (strncmp(cmd, "[CENTRAL_TASK]", 14) == 0) {
        char t_name[50] = {0};
        char t_target[20] = {0};
        int t_phase = 1;

        // ???? ???? ???? ???? ???? ???? ?? sscanf
        char* task_ptr = strstr(cmd, "Task:");
        char* target_ptr = strstr(cmd, "|Target:");
        char* phase_ptr = strstr(cmd, "|Phase:");

        if (task_ptr && target_ptr && phase_ptr) {
            task_ptr += 5; // ?? ??? ?? ???? "Task:"
            int name_len = target_ptr - task_ptr;
            if (name_len < 50) {
                strncpy(t_name, task_ptr, name_len);
                t_name[name_len] = '\0';
            }

            target_ptr += 8; // ?? ??? ?? ???? "|Target:"
            int target_len = phase_ptr - target_ptr;
            if (target_len < 20) {
                strncpy(t_target, target_ptr, target_len);
                t_target[target_len] = '\0';
            }

            phase_ptr += 7; // ?? ??? ?? ???? "|Phase:"
            t_phase = atoi(phase_ptr);

            // ????? ? ?????? ??? ?? ?? ???????
            bool found = false;
            for (int i = 0; i < 2; i++) {
                if (lcd_queue[i].is_valid && strcmp(lcd_queue[i].name, t_name) == 0) {
                    lcd_queue[i].phase = t_phase;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                for (int i = 0; i < 2; i++) {
                    if (!lcd_queue[i].is_valid) {
                        strcpy(lcd_queue[i].name, t_name);
                        strcpy(lcd_queue[i].target, t_target);
                        lcd_queue[i].phase = t_phase;
                        lcd_queue[i].is_valid = true;
                        break;
                    }
                }
            }
            lcd_needs_update = true; // ??? ?? ???? ???% ???? ????
        }
    }
    else if (strncmp(cmd, "[CENTRAL_OK]", 12) == 0) {
        is_worker_screen_active = true;
        worker_screen_timer = HAL_GetTick();
        lcd_needs_update = true;
    }
    else if (strncmp(cmd, "[CENTRAL_REJECT]", 16) == 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)"[SYSTEM]: Access Denied! Not your group phase.\r\n", 48, 100);
    }
    else if (strncmp(cmd, "[CENTRAL] TIMEOUT_ASSIGNED", 26) == 0) {
        char t_name[50] = {0};
        char* assign_ptr = strstr(cmd, "ASSIGNED:");
        if (assign_ptr) {
            assign_ptr += 9;
            char* colon_ptr = strchr(assign_ptr, ':');
            if (colon_ptr) {
                int n_len = colon_ptr - assign_ptr;
                strncpy(t_name, assign_ptr, n_len);
                t_name[n_len] = '\0';
                
                for (int i = 0; i < 2; i++) {
                    if (lcd_queue[i].is_valid && strcmp(lcd_queue[i].name, t_name) == 0) {
                        snprintf(worker_id_to_show, sizeof(worker_id_to_show), "RND: Active");
                        worker_screen_line = i;
                        is_worker_screen_active = true;
                        worker_screen_timer = HAL_GetTick();
                        break;
                    }
                }
                lcd_needs_update = true;
            }
        }
    }
}

void Shift_Queue_Up(int line_to_clear) {
    if (line_to_clear == 0) {
        lcd_queue[0] = lcd_queue[1];
        lcd_queue[1].is_valid = false;
    } else if (line_to_clear == 1) {
        lcd_queue[1].is_valid = false;
    }
}

void Refresh_LCD(void) {
    // ?. ????? ????? ???? ????
    if (is_error_active) {
        char line1[32];
        char line2[32];
        
        LCD_Clear();
        
        // ??? ?? ???: Error: ID
        snprintf(line1, sizeof(line1), "Error: %s", current_error_id);
        LCD_Goto(1, 1);
        LCD_Print(line1);
        
        // ??? ?? ???: [Error name]
        snprintf(line2, sizeof(line2), "[%s]", current_error_action);
        LCD_Goto(2, 1);
        LCD_Print(line2);
        
        return;
    }

    // ?. ????? ????? ???? "??? ?? ??"
    if (is_error_solved_screen) {
        LCD_Clear();
        LCD_Goto(1, 1);
        LCD_Print("Error Solved!");
        return;
    }

    // ?. ???? ???? (????? ?????? ?? ???????)
    LCD_Clear();
    bool has_task = false; // ?????? ???? ????? ???? ??? ????

    for (int i = 0; i < 2; i++) {
        // ??? ?????? ??? ?? i ??? ??? ????
        if (is_worker_screen_active && worker_screen_line == i) {
            char display_str[32];
            snprintf(display_str, sizeof(display_str), "Worker ID: %s", worker_id_to_show);
            LCD_Goto(i + 1, 1);
            LCD_Print(display_str);
            has_task = true;
        }
        // ??? ???? ?? ?? ?? i ???? ????? ????
        else if (lcd_queue[i].is_valid) {
            char display_str[32];
            int grp_num = lcd_queue[i].target[7] - '0'; 
            
            if (lcd_queue[i].phase == 1) {
                snprintf(display_str, sizeof(display_str), "%s - %d", lcd_queue[i].name, grp_num);
            }
            else if (lcd_queue[i].phase == 2) {
                int second_grp = (grp_num == 3) ? 4 : (grp_num == 2 ? 3 : 3);
                snprintf(display_str, sizeof(display_str), "%s - %d,%d", lcd_queue[i].name, grp_num, second_grp);
            }
            else if (lcd_queue[i].phase == 3) {
                snprintf(display_str, sizeof(display_str), "%s - 3,4,2", lcd_queue[i].name);
            }
            else if (lcd_queue[i].phase == 4) {
                snprintf(display_str, sizeof(display_str), "%s - X", lcd_queue[i].name); 
            }
            
            LCD_Goto(i + 1, 1);
            LCD_Print(display_str);
            has_task = true;
        }
    }

    // ?. ???? ??????? (??? ??? ? ???? ???? ?????)
    if (!has_task) {
        LCD_Goto(1, 1);
        LCD_Print("Waiting Tasks...");
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
