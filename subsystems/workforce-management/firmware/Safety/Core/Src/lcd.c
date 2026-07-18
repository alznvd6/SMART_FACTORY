#include "lcd.h"

// تابع کمکی برای ارسال 4 بیت (Nibble) به پورت‌های ترکیبی A و B
static void LCD_Write_Nibble(uint8_t nibble) {
    // تفکیک بیت‌ها و ارسال به پین‌های مربوطه بر اساس تنظیمات CubeMX
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // ایجاد پالس Enable برای ثبت دیتا در ال‌سی‌دی
    HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // یک میلی‌ثانیه تاخیر برای پایداری در پروتئوس
    HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

// تابع کمکی برای ارسال دستور (is_data = 0) یا کاراکتر (is_data = 1)
static void LCD_Send(uint8_t value, uint8_t is_data) {
    // تعیین حالت RS
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, is_data ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // در حالت ۴ بیتی، ابتدا ۴ بیت با ارزش بالا و سپس ۴ بیت با ارزش پایین ارسال می‌شود
    LCD_Write_Nibble(value >> 4);   // ارسال ۴ بیت بالا
    LCD_Write_Nibble(value & 0x0F); // ارسال ۴ بیت پایین
}

// مقداردهی اولیه ال‌سی‌دی (استاندارد HD44780 برای مد 4 بیتی)
void LCD_Init(void) {
    HAL_Delay(50); // صبر برای پایداری ولتاژ در لحظه روشن شدن
    
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(E_GPIO_Port, E_Pin, GPIO_PIN_RESET);

    // توالی راه‌اندازی سخت‌افزاری (بسیار مهم برای پروتئوس)
    LCD_Write_Nibble(0x03);
    HAL_Delay(5);
    LCD_Write_Nibble(0x03);
    HAL_Delay(1);
    LCD_Write_Nibble(0x03);
    HAL_Delay(1);
    
    // تنظیم روی حالت 4 بیتی
    LCD_Write_Nibble(0x02); 
    HAL_Delay(1);

    // تنظیمات نمایشگر
    LCD_Send(0x28, 0); // مد 4 بیتی، 2 سطر، فونت 5x8
    LCD_Send(0x0C, 0); // روشن کردن نمایشگر، خاموش کردن مکان‌نما (Cursor)
    LCD_Send(0x06, 0); // حرکت اتوماتیک مکان‌نما به جلو پس از هر کاراکتر
    
    LCD_Clear();
}

// پاک کردن کل صفحه نمایش
void LCD_Clear(void) {
    LCD_Send(0x01, 0);
    HAL_Delay(2); // دستور پاک کردن نیاز به زمان بیشتری دارد
}

// انتقال مکان‌نما به سطر و ستون دلخواه (سطر: 1 یا 2، ستون: 1 تا 16)
void LCD_Goto(uint8_t row, uint8_t col) {
    uint8_t address;
    if (row == 1) {
        address = 0x80; // آدرس شروع سطر اول
    } else {
        address = 0xC0; // آدرس شروع سطر دوم
    }
    address += (col - 1);
    LCD_Send(address, 0);
}

// چاپ رشته کاراکتری روی صفحه
void LCD_Print(char *str) {
    while (*str) {
        LCD_Send((uint8_t)(*str), 1);
        str++;
    }
}