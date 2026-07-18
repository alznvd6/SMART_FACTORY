#ifndef LCD_H
#define LCD_H

#include "main.h" // برای دسترسی به تعاریف پین‌ها و HAL

// توابع اصلی که در main.c فراخوانی شده‌اند
void LCD_Init(void);
void LCD_Clear(void);
void LCD_Goto(uint8_t row, uint8_t col);
void LCD_Print(char *str);

#endif // LCD_H