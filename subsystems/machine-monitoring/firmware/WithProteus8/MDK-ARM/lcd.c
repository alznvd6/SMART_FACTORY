#include "lcd.h"

// ???? ??? ??????
#define LCD_RS_PIN    GPIO_PIN_3
#define LCD_EN_PIN    GPIO_PIN_4
#define LCD_RS_PORT   GPIOB
#define LCD_EN_PORT   GPIOB

// ???? ??? ????
#define LCD_D4_PIN    GPIO_PIN_5
#define LCD_D5_PIN    GPIO_PIN_6
#define LCD_D6_PIN    GPIO_PIN_7
#define LCD_D7_PIN    GPIO_PIN_8
#define LCD_DATA_PORT GPIOB

// ???? ???? EN ???? ????? ????
static void LCD_Pulse(void)
{
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

// ????? 4 ??? ?? LCD
static void LCD_Send4Bit(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D4_PIN, (data >> 0) & 1);
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D5_PIN, (data >> 1) & 1);
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D6_PIN, (data >> 2) & 1);
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D7_PIN, (data >> 3) & 1);
    LCD_Pulse();
}

// ????? ?? ???? ???? (????? ?? ????)
static void LCD_Send(uint8_t data, uint8_t isData)
{
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, isData);
    LCD_Send4Bit(data >> 4);   // nibble ????
    LCD_Send4Bit(data & 0x0F); // nibble ?????
}

// ???????? ????? LCD
void LCD_Init(void)
{
    HAL_Delay(50);
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);

    // ????? 4-bit mode
    LCD_Send4Bit(0x03); HAL_Delay(5);
    LCD_Send4Bit(0x03); HAL_Delay(1);
    LCD_Send4Bit(0x03); HAL_Delay(1);
    LCD_Send4Bit(0x02); HAL_Delay(1);

    LCD_Send(0x28, 0); // 4-bit, 2 ??, 5x8
    LCD_Send(0x0C, 0); // ????? ????? ???????? ?????
    LCD_Send(0x06, 0); // ?????? ??????
    LCD_Send(0x01, 0); // ??? ????
    HAL_Delay(2);
}

// ??? ???? ????
void LCD_Clear(void)
{
    LCD_Send(0x01, 0);
    HAL_Delay(2);
}

// ????? ?????? ????????
void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t pos = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_Send(pos, 0);
}

// ????? ???
void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_Send((uint8_t)(*str), 1);
        str++;
    }
}