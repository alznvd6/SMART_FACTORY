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
    STATE_WAIT_SERVER_AUTH, 
    STATE_WAIT_WORKER_MENU_ACTION,
    
    STATE_MANAGER_USER_PROMPT,
    STATE_MANAGER_WAIT_USER,
    STATE_MANAGER_PASS_PROMPT,
    STATE_MANAGER_WAIT_PASS,
    STATE_WAIT_MANAGER_MENU_ACTION,
    STATE_MANAGER_WAIT_ABSENCE_ID,
    STATE_MANAGER_WAIT_FORCE_OUT_ID,
    
    STATE_WAIT_EMERGENCY_CHOICE,
    STATE_WAIT_MEDICAL_CHOICE,
    STATE_WORKER_WAIT_LETTER_TEXT,   
    STATE_MANAGER_WAIT_REPORT_TEXT,  
    STATE_MANAGER_WAIT_DEMAND_TEXT,
		STATE_MANAGER_WAIT_INFO_ID,
} SystemState;

SystemState currentState = STATE_MAIN_MENU;

// ???????? ??????? ????? (UART1)
char term_rx_buffer[100];
uint8_t term_rx_index = 0;
uint8_t term_rx_data;
volatile uint8_t term_input_ready = 0;

// ???????? ???? ?????? (UART3)
char srv_rx_buffer[100];
uint8_t srv_rx_index = 0;
uint8_t srv_rx_data;
volatile uint8_t srv_input_ready = 0;

const char admin_user[] = "admin";
const char admin_pass[] = "1234";
char entered_user[20];
char entered_pass[20];
char entered_name[30]; 

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
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_Send4Bit(uint8_t data) {
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LCD_Enable(void) {
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void LCD_Cmd(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET); // RS = 0 ???? ?????
    LCD_Send4Bit(cmd >> 4);
    LCD_Enable();
    LCD_Send4Bit(cmd & 0x0F);
    LCD_Enable();
}

void LCD_Data(uint8_t data) {
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET); // RS = 1 ???? ????
    LCD_Send4Bit(data >> 4);
    LCD_Enable();
    LCD_Send4Bit(data & 0x0F);
    LCD_Enable();
}

void LCD_Init(void) {
    HAL_Delay(50);
    LCD_Cmd(0x33);
    LCD_Cmd(0x32);
    LCD_Cmd(0x28); // ???? 4 ???
    LCD_Cmd(0x0C); // ???? ???? ??????? ???? ??????
    LCD_Cmd(0x01); // ??? ???? ????
    HAL_Delay(2);
}

void LCD_String(char* str) {
    while (*str) {
        LCD_Data(*str++);
    }
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t pos = (row == 0) ? 0x80 : 0xC0;
    LCD_Cmd(pos + col);
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
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &term_rx_data, 1);
  HAL_UART_Receive_IT(&huart3, &srv_rx_data, 1); 
  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
  LCD_Init();
  LCD_SetCursor(0, 0);
  LCD_String("  SMART FACTORY  ");
  LCD_SetCursor(1, 0);
  LCD_String(" STATUS: NORMAL  ");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
    if (srv_input_ready) {
            srv_input_ready = 0;
            trim_string(srv_rx_buffer);

            if (srv_rx_buffer[0] == '@') {
                if (strncmp(srv_rx_buffer, "@NAME:", 6) == 0) {
                    strcpy(entered_name, srv_rx_buffer + 6);
                    printf("\r\n========================================");
                    printf("\r\n[ACCESS GRANTED] Welcome: %s", entered_name);
                    printf("\r\n========================================\r\n");
                    printf("--- WORKER MENU ---\r\n");
                    printf("1. Clock In (Check-in)\r\n2. Clock Out (Check-out)\r\n3. Request Leave (Vacation)\r\n4. Send Letter\r\n5. Logout\r\nSelect: ");
                    
                    LCD_Cmd(0x01);
                    LCD_SetCursor(0, 0); LCD_String("Welcome Worker:");
                    LCD_SetCursor(1, 0); 
                    char lcd_buf[17];
                    sprintf(lcd_buf, "%-16.16s", entered_name);
                    LCD_String(lcd_buf);
                    currentState = STATE_WAIT_WORKER_MENU_ACTION;
                }
                else if (strncmp(srv_rx_buffer, "@MSG:", 5) == 0) {
                    printf("\r\n[SERVER]: %s\r\n", srv_rx_buffer + 5);
                    if (currentState == STATE_WAIT_SERVER_AUTH) {
                        currentState = STATE_MAIN_MENU; 
                    } else if (currentState == STATE_WAIT_WORKER_MENU_ACTION) {
                        printf("\r\nSelect next option (or 5 to logout): ");
                    } else if (currentState == STATE_WAIT_MANAGER_MENU_ACTION) {
                        printf("\r\nSelect next option (or 7 to logout): ");
                    }
                }
								else if (strncmp(srv_rx_buffer, "@PRT:", 5) == 0) {
                    printf("\r\n%s", srv_rx_buffer + 5);
                }
                else if (strncmp(srv_rx_buffer, "@LCD1:", 6) == 0) {
                    LCD_SetCursor(0, 0);
                    char lcd_buf[17];
                    sprintf(lcd_buf, "%-16.16s", srv_rx_buffer + 6);
                    LCD_String(lcd_buf);
                }
                else if (strncmp(srv_rx_buffer, "@LCD2:", 6) == 0) {
                    LCD_SetCursor(1, 0);
                    char lcd_buf[17];
                    sprintf(lcd_buf, "%-16.16s", srv_rx_buffer + 6);
                    LCD_String(lcd_buf);
                }
            }
            srv_rx_index = 0; // <--- ???? ???: ???? ???? ????? ???? ???????? ????
        }

        // 2. ?????? ????? ?????? ?? ??????? ?????
        if (term_input_ready) {
            term_input_ready = 0;
            trim_string(term_rx_buffer);
            char tx_buf[50]; 

            switch (currentState) {
                case STATE_WAIT_MAIN_CHOICE:
                    if (term_rx_buffer[0] == '1') {
                        printf("\r\n\r\n[WORKER LOGIN] Please enter Fingerprint ID: ");
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Worker Login:"); LCD_SetCursor(1,0); LCD_String("Waiting for ID..");
                        currentState = STATE_WAIT_WORKER_ID;
                    } else if (term_rx_buffer[0] == '2') {
                        printf("\r\n\r\n[MANAGER LOGIN]\r\nUsername: ");
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Manager Login:"); LCD_SetCursor(1,0); LCD_String("Enter Username..");
                        currentState = STATE_MANAGER_WAIT_USER;
                    } else if (term_rx_buffer[0] == '3') {
                        printf("\r\n\r\n!!! EMERGENCY PROTOCOL !!!\r\n1. Fire\r\n2. Medical\r\n3. Evacuate\r\n4. Cancel\r\nSelect: ");
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Emergency Menu"); LCD_SetCursor(1,0); LCD_String("Select Type...");
                        currentState = STATE_WAIT_EMERGENCY_CHOICE; 
                    } else {
                        printf("\r\nInvalid choice. Try again: ");
                    }
                    break;

                case STATE_WAIT_WORKER_ID:
                    strcpy(entered_user, term_rx_buffer); 
                    printf("\r\n[SYSTEM] Verifying ID %s with Server...\r\n", entered_user);
                    LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Authenticating.."); 
                    
                    sprintf(tx_buf, "REQ_NAME:%s\r\n", entered_user);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    
                    currentState = STATE_WAIT_SERVER_AUTH;
                    break;

                case STATE_WAIT_WORKER_MENU_ACTION:
                    if (term_rx_buffer[0] == '1') {
                        sprintf(tx_buf, "CHECKIN:%s\r\n", entered_user);
                        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    } 
                    else if (term_rx_buffer[0] == '2') {
                        sprintf(tx_buf, "CHECKOUT:%s\r\n", entered_user);
                        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    }
                    else if (term_rx_buffer[0] == '3') {
                        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
                            sprintf(tx_buf, "VACATION:%s:APPROVED\r\n", entered_user);
                            HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                            printf("\r\n[RESULT] Vacation APPROVED by Manager Panel.\r\nSelect next option (or 5 to logout): ");
                        } else {
                            sprintf(tx_buf, "VACATION:%s:REJECTED\r\n", entered_user);
                            HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                            printf("\r\n[RESULT] Vacation REJECTED by Manager Panel.\r\nSelect next option (or 5 to logout): ");
                        }
                    }
                    else if (term_rx_buffer[0] == '4') {
                        printf("\r\nPlease type your message/letter: ");
                        currentState = STATE_WORKER_WAIT_LETTER_TEXT;
                    }
                    else if (term_rx_buffer[0] == '5') {
                        printf("\r\n[LOGGED OUT] Returning to Main Menu...\r\n");
                        currentState = STATE_MAIN_MENU;
                    } else {
                        printf("\r\nInvalid option.\r\nSelect next option (or 5 to logout): ");
                    }
                    break;

                case STATE_MANAGER_WAIT_USER:
                    strcpy(entered_user, term_rx_buffer);
                    printf("\r\nPassword: ");
                    LCD_SetCursor(1,0); LCD_String("Enter Password..");
                    currentState = STATE_MANAGER_WAIT_PASS;
                    break;

                case STATE_MANAGER_WAIT_PASS:
                    strcpy(entered_pass, term_rx_buffer);
                    if (strcmp(entered_user, admin_user) == 0 && strcmp(entered_pass, admin_pass) == 0) {
                        printf("\r\n========================================");
                        printf("\r\n[ACCESS GRANTED] Welcome Foreman/Manager");
                        printf("\r\n========================================\r\n");
                        printf("--- MANAGER MENU ---\r\n");
                        printf("1. Stop Production Line\r\n2. Report Failure\r\n3. Send Demand/Letter\r\n4. Register Absence\r\n5. Force Worker Checkout\r\n6. View Worker Info\r\n7. Logout\r\nSelect: ");
                        
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Manager Panel"); LCD_SetCursor(1,0); LCD_String("Access Granted!");
                        currentState = STATE_WAIT_MANAGER_MENU_ACTION;
                    } else {
                        printf("\r\n[ACCESS DENIED] Incorrect Username or Password.\r\n");
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Access Denied!"); 
                        currentState = STATE_MAIN_MENU;
                    }
                    break;

                case STATE_WAIT_MANAGER_MENU_ACTION:
                    if (term_rx_buffer[0] == '1') {
                        sprintf(tx_buf, "MANAGER:STOP_LINE\r\n");
                        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                        printf("\r\n[ALARM] Production Line STOPPED command sent!\r\nSelect next option (or 7 to logout): ");
                    }
                    else if (term_rx_buffer[0] == '2') {
                        printf("\r\nEnter failure details: ");
                        currentState = STATE_MANAGER_WAIT_REPORT_TEXT;
                    }
                    else if (term_rx_buffer[0] == '3') {
                        printf("\r\nEnter demand/task details: ");
                        currentState = STATE_MANAGER_WAIT_DEMAND_TEXT;
                    }
                    else if (term_rx_buffer[0] == '4') {
                        printf("\r\nEnter Worker ID to mark as Absent (e.g. 1001): ");
                        currentState = STATE_MANAGER_WAIT_ABSENCE_ID;
                    }
                    else if (term_rx_buffer[0] == '5') {
                        printf("\r\nEnter Worker ID to Force Checkout (e.g. 1001): ");
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Force Checkout:"); LCD_SetCursor(1,0); LCD_String("Enter ID:       ");
                        currentState = STATE_MANAGER_WAIT_FORCE_OUT_ID;
                    }
                    else if (term_rx_buffer[0] == '6') {  // <--- ???? ????? ? ?? ???? ???????
                        printf("\r\nEnter Worker ID to view info (e.g. 1001): ");
                        LCD_Cmd(0x01); LCD_SetCursor(0,0); LCD_String("Worker Info:"); LCD_SetCursor(1,0); LCD_String("Enter ID:       ");
                        currentState = STATE_MANAGER_WAIT_INFO_ID;
                    }
                    else if (term_rx_buffer[0] == '7') {  // <--- ???? ????? ? ?? ????
                        printf("\r\n[LOGGED OUT] Returning to Main Menu...\r\n");
                        currentState = STATE_MAIN_MENU;
                    } 
                    else {
                        printf("\r\nInvalid option.\r\nSelect next option (or 7 to logout): ");
                    }
                    break;

                case STATE_MANAGER_WAIT_FORCE_OUT_ID:
                    sprintf(tx_buf, "FORCE_OUT:%s\r\n", term_rx_buffer);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    currentState = STATE_WAIT_MANAGER_MENU_ACTION;
                    break;

                case STATE_WORKER_WAIT_LETTER_TEXT:
                    sprintf(tx_buf, "LETTER:%s:%s\r\n", entered_user, term_rx_buffer);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    printf("\r\n[RESULT] Your letter was sent to the server.\r\nSelect next option (or 5 to logout): ");
                    currentState = STATE_WAIT_WORKER_MENU_ACTION;
                    break;

                case STATE_MANAGER_WAIT_REPORT_TEXT:
                    sprintf(tx_buf, "MANAGER:REPORT_FAIL:%s\r\n", term_rx_buffer);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    printf("\r\n[SYSTEM] Failure report sent to database.\r\nSelect next option (or 7 to logout): ");
                    currentState = STATE_WAIT_MANAGER_MENU_ACTION;
                    break;

                case STATE_MANAGER_WAIT_DEMAND_TEXT:
                    sprintf(tx_buf, "MANAGER:DEMAND:%s\r\n", term_rx_buffer);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    printf("\r\n[SYSTEM] Demand letter sent to database.\r\nSelect next option (or 7 to logout): ");
                    currentState = STATE_WAIT_MANAGER_MENU_ACTION;
                    break;

                case STATE_MANAGER_WAIT_ABSENCE_ID:
                    sprintf(tx_buf, "ABSENCE:%s\r\n", term_rx_buffer);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
                    printf("\r\n[SYSTEM] Absence record sent for ID: %s\r\nSelect next option (or 7 to logout): ", term_rx_buffer);
                    currentState = STATE_WAIT_MANAGER_MENU_ACTION;
                    break;
                    
                case STATE_MANAGER_WAIT_INFO_ID:
                    sprintf(tx_buf, "MANAGER:INFO:%s\r\n", term_rx_buffer);
                    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buf, strlen(tx_buf), 100);
										printf("\r\n[SYSTEM] Fetching data from server...\r\n");
                    currentState = STATE_WAIT_MANAGER_MENU_ACTION; 
                    break;
								case STATE_WAIT_EMERGENCY_CHOICE:
                    if (term_rx_buffer[0] == '1') {
                        printf("\r\nEMERGENCY:FIRE\r\n");
                        printf("\r\n[ALARM] Fire Alarm triggered! Strobe Lights & Buzzer Activated.\r\n");
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
                        printf("\r\nReturning to Main Menu...\r\n");
                        currentState = STATE_MAIN_MENU;
                    } 
                    else if (term_rx_buffer[0] == '2') {
                        printf("\r\n\r\n--- MEDICAL EMERGENCY SUBMENU ---\r\n");
                        printf("1. Machinery Accident / Injury\r\n");
                        printf("2. Chemical / Toxic Exposure\r\n");
                        printf("3. Back to Main Menu\r\n");
                        printf("Select Medical Type: ");
                        currentState = STATE_WAIT_MEDICAL_CHOICE;
                    } 
                    else if (term_rx_buffer[0] == '3') { 
                        printf("\r\nEMERGENCY:CRITICAL_EVACUATE\r\n");
                        printf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
                        printf("!! CRITICAL SITUATION: EVACUATE IMMEDIATELY !!\r\n");
                        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
                        
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);  
                        
                        LCD_Cmd(0x01);
                        LCD_SetCursor(0, 0); LCD_String("!!! EMERGENCY !!!");
                        LCD_SetCursor(1, 0); LCD_String("EVACUATE FACTORY!");
                        printf("\r\nReturning to Main Menu...\r\n");
                        currentState = STATE_MAIN_MENU;
                    } 
                    else if (term_rx_buffer[0] == '4') {
                        printf("\r\nReturning to Main Menu...\r\n");
                        currentState = STATE_MAIN_MENU;
                    } 
                    else {
                        printf("\r\nInvalid emergency type.\r\nSelect: ");
                    }
                    break;

                case STATE_WAIT_MEDICAL_CHOICE:
                    if (term_rx_buffer[0] == '1') {
                        printf("\r\nEMERGENCY:MEDICAL_MACHINERY\r\n");
                        printf("\r\n[NOTIFICATION] First Aid Team dispatched for Machinery Accident.\r\n");
                    } 
                    else if (term_rx_buffer[0] == '2') {
                        printf("\r\nEMERGENCY:MEDICAL_CHEMICAL\r\n");
                        printf("\r\n[NOTIFICATION] Chemical Exposure protocol activated. Hazmat team notified.\r\n");
                    } 
                    else {
                        printf("\r\nReturning without dispatch.\r\n");
                    }
                    currentState = STATE_MAIN_MENU;
                    break;
            }
            term_rx_index = 0; // <--- ???? ???: ???? ???? ????? ?????? ???? ??????? ????
        }

        // 3. ?????? ?? ???? ????
        if (currentState == STATE_MAIN_MENU) {
            LCD_Cmd(0x01);
            LCD_SetCursor(0, 0); LCD_String("  SMART FACTORY  ");
            LCD_SetCursor(1, 0); LCD_String(" STATUS: NORMAL  ");
            
            printf("\r\n\r\n=== SMART FACTORY CENTRAL TERMINAL ===\r\n");
            printf("1. Worker Login (Fingerprint ID)\r\n");
            printf("2. Foreman/Manager Login (Credentials)\r\n");
            printf("3. EMERGENCY (SOS)\r\n");
            printf("Select an option (1, 2, or 3): ");
            currentState = STATE_WAIT_MAIN_CHOICE;
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
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, LCD_RS_Pin|LCD_EN_Pin|LCD_D4_Pin|LCD_D5_Pin
                          |LCD_D6_Pin|LCD_D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Fire_LEDs_Pin|Critical_Alarm_Lights_Pin|Buzzer_Pin|Relay_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : FINGERPRINT_SENSOR_Pin */
  GPIO_InitStruct.Pin = FINGERPRINT_SENSOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FINGERPRINT_SENSOR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RS_Pin LCD_EN_Pin LCD_D4_Pin LCD_D5_Pin
                           LCD_D6_Pin LCD_D7_Pin */
  GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_EN_Pin|LCD_D4_Pin|LCD_D5_Pin
                          |LCD_D6_Pin|LCD_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : REQUEST_STATUS_Pin */
  GPIO_InitStruct.Pin = REQUEST_STATUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(REQUEST_STATUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Fire_LEDs_Pin Critical_Alarm_Lights_Pin Buzzer_Pin Relay_Pin */
  GPIO_InitStruct.Pin = Fire_LEDs_Pin|Critical_Alarm_Lights_Pin|Buzzer_Pin|Relay_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
        if (term_rx_data == '\r' || term_rx_data == '\n') {
            if (term_rx_index > 0) { 
                term_rx_buffer[term_rx_index] = '\0';
                term_input_ready = 1;
            }
            // ????? ?????: ???? ???? ?? ?? ???? ??? ?? ?? ?????
            uint8_t nl[] = {'\r', '\n'};
            HAL_UART_Transmit(&huart1, nl, 2, 10);
        } 
        // ???? ????????: ???????? ?? ?? ?? ??????? ????????? ? 0x7F ???????
        else if (term_rx_data == '\b' || term_rx_data == 0x08 || term_rx_data == 0x7F) { 
            if (term_rx_index > 0) {
                term_rx_index--; 
                uint8_t back_seq[] = {'\b', ' ', '\b'}; 
                HAL_UART_Transmit(&huart1, back_seq, 3, 10);
            }
        } 
        else {
            if (term_rx_index < sizeof(term_rx_buffer) - 1) {
                term_rx_buffer[term_rx_index++] = term_rx_data;
                HAL_UART_Transmit(&huart1, &term_rx_data, 1, 10); 
            }
        }
        HAL_UART_Receive_IT(&huart1, &term_rx_data, 1);
    }
    
    else if (huart->Instance == USART3) {
        if (srv_rx_data == '\r' || srv_rx_data == '\n') {
            if (srv_rx_index > 0) {
                srv_rx_buffer[srv_rx_index] = '\0';
                srv_input_ready = 1;
            }
        } 
        else {
            // ??? ?????: ??? ?? ????? ??? ???? ?? ???? ?? ???? ???? ???? ?????? ??? ????
            if (srv_input_ready == 0) {
                if (srv_rx_index < sizeof(srv_rx_buffer) - 1) {
                    srv_rx_buffer[srv_rx_index++] = srv_rx_data;
                }
            }
        }
        HAL_UART_Receive_IT(&huart3, &srv_rx_data, 1);
    }
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // ??? ???? ??? ??????????? ? ?????????? ???? ?????? ?? ???? ???? ???? Overrun
    if (huart->Instance == USART1) {
        // ??? ???? ???? ??? ?? ???? ??????????
        __HAL_UART_CLEAR_OREFLAG(huart);
        // ???? ???? ?????? ????????? ???????
        HAL_UART_Receive_IT(&huart1, &term_rx_data, 1);
    }
    else if (huart->Instance == USART3) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        // ???? ???? ?????? ????????? ???? ??????
        HAL_UART_Receive_IT(&huart3, &srv_rx_data, 1);
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
