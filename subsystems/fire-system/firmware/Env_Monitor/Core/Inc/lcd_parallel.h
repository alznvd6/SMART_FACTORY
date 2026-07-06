/*
 * liquidcrystal_i2c.h
 *
 *  Created on: Jul 6, 2026
 *      Author: Navid
 */
#ifndef INC_LCD_PARALLEL_H_
#define INC_LCD_PARALLEL_H_

#include "stm32f1xx_hal.h"

// Define GPIO Port for control and data pins
#define LCD_CTRL_PORT  GPIOB
#define LCD_DATA_PORT  GPIOB

// Pin assignments
#define LCD_RS_PIN     GPIO_PIN_10
#define LCD_E_PIN      GPIO_PIN_11
#define LCD_D4_PIN     GPIO_PIN_12
#define LCD_D5_PIN     GPIO_PIN_13
#define LCD_D6_PIN     GPIO_PIN_14
#define LCD_D7_PIN     GPIO_PIN_15

// Function prototypes
void lcd_init(void);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_clear(void);
void lcd_put_cur(int row, int col);

#endif /* INC_LCD_PARALLEL_H_ */
