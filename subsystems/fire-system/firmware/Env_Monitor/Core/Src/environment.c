/*
 * environment.c
 *
 *  Created on: Jul 5, 2026
 *      Author: Navid
 */

/*
 * environment.c
 *
 * Created on: Jul 5, 2026
 * Author: Navid
 */
#include "environment.h"
#include <stdio.h>
#include <string.h>

// Private pointers to peripherals
static ADC_HandleTypeDef *env_hadc;
static UART_HandleTypeDef *env_huart;

// Circular Buffer Variables for Moving Average
static float temp_history[MOVING_AVG_SAMPLES] = {0};
static int buffer_index = 0;
static int total_samples_collected = 0;

// Fire Safety Lock-out State tracking
static uint32_t last_fire_detected_time = 0;
static uint8_t is_system_locked = 0;

// Private Helper Functions
static void Add_Temperature_To_History(float new_temp);
static float Get_Average_Temperature(void);
static void Control_Temperature_Actuators(float avg_temp);
static void Control_Fire_Safety(char *fire_status_msg, size_t msg_max_len);

// Public: Initialize module
void ENV_Init(ADC_HandleTypeDef *hadc_ptr, UART_HandleTypeDef *huart_ptr) {
    env_hadc = hadc_ptr;
    env_huart = huart_ptr;
}

// Public: Core task run in main loop
void ENV_Task(void) {
    uint32_t adc_value = 0;
    float current_temperature = 0.0f;
    float average_temperature = 0.0f;

    char fire_msg_buffer[40] = "SYSTEM SAFE";
    char uart_buffer[160];

    // 1. Measure Temperature
    HAL_ADC_Start(env_hadc);
    if (HAL_ADC_PollForConversion(env_hadc, 10) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(env_hadc);
        current_temperature = ((float)adc_value * 330.0f) / 4095.0f;
    }
    HAL_ADC_Stop(env_hadc);

    // 2. Process Data
    Add_Temperature_To_History(current_temperature);
    average_temperature = Get_Average_Temperature();

    // 3. Act on Data Subsystems
    Control_Temperature_Actuators(average_temperature);
    Control_Fire_Safety(fire_msg_buffer, sizeof(fire_msg_buffer));

    // 4. Output Status to Virtual Terminal (Only displaying Current + Avg + Safety status)
    snprintf(uart_buffer, sizeof(uart_buffer),
             "Current Temp: %.2f C | Avg Temp: %.2f C | Fire State: %s\r\n",
             current_temperature, average_temperature, fire_msg_buffer);
    HAL_UART_Transmit(env_huart, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
}

// Private: Updates the circular buffer
static void Add_Temperature_To_History(float new_temp) {
    temp_history[buffer_index] = new_temp;
    buffer_index = (buffer_index + 1) % MOVING_AVG_SAMPLES;

    if (total_samples_collected < MOVING_AVG_SAMPLES) {
        total_samples_collected++;
    }
}

// Private: Computes the average of available readings
static float Get_Average_Temperature(void) {
    float sum = 0.0f;
    for (int i = 0; i < total_samples_collected; i++) {
        sum += temp_history[i];
    }
    return (total_samples_collected > 0) ? (sum / (float)total_samples_collected) : 0.0f;
}

// Private: Controls Fan and Heater logic
static void Control_Temperature_Actuators(float avg_temp) {
    if (avg_temp > HIGH_TEMP_THRESHOLD) {
        HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_RESET);
    }
    else if (avg_temp < LOW_TEMP_THRESHOLD) {
        HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_RESET);
    }
}

static void Control_Fire_Safety(char *fire_status_msg, size_t msg_max_len) {
    uint8_t fire1 = (HAL_GPIO_ReadPin(FIRE_1_PORT, FIRE_1_PIN) == GPIO_PIN_SET);
    uint8_t fire2 = (HAL_GPIO_ReadPin(FIRE_2_PORT, FIRE_2_PIN) == GPIO_PIN_SET);
    uint32_t current_time = HAL_GetTick();

    if (fire1 || fire2) {
        last_fire_detected_time = current_time;
        is_system_locked = 1;
    }
    else if (is_system_locked && (current_time - last_fire_detected_time >= ALARM_LOCK_DURATION_MS)) {
        is_system_locked = 0;
    }

    if (is_system_locked) {
        HAL_GPIO_WritePin(SPRINKLER_1_PORT, SPRINKLER_1_PIN, fire1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SPRINKLER_2_PORT, SPRINKLER_2_PIN, fire2 ? GPIO_PIN_SET : GPIO_PIN_RESET);

        // --- SOUNDER FIX: TOGGLE PIN TO CREATE AN AUDIO WAVE ---
        // This will invert the pin state every 1 second (since ENV_Task runs every 1000ms in main.c)
        // Creating a clear, repetitive beeping alarm tone.
        HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);

        snprintf(fire_status_msg, msg_max_len, "!!! FIRE DETECTED !!!");
    } else {
        HAL_GPIO_WritePin(SPRINKLER_1_PORT, SPRINKLER_1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SPRINKLER_2_PORT, SPRINKLER_2_PIN, GPIO_PIN_RESET);

        // Explicitly turn off the sounder when safe
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        snprintf(fire_status_msg, msg_max_len, "CLEAR");
    }
}
