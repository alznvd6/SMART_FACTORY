/*
 * liquidcrystal_i2c.c
 *
 *  Created on: Jul 6, 2026
 *      Author: Navid
 */


/*
 * liquidcrystal_i2c.c
 * Modified for 20x4 Parallel LCD with Custom CGRAM Fire Logo
 */

#include "lcd_parallel.h" // keeps original parallel includes

// Fire icon custom bitmap (5x8 matrix representation)
static const uint8_t fire_frame_A[8] = {
    0x04,  //    *
    0x04,  //    *
    0x0A,  //   * *
    0x0A,  //   * *
    0x15,  //  * * *
    0x15,  //  * * *
    0x1F,  // *******
    0x0E   //  ***
};

// Frame 2: Dynamic flame shape B (Flicker state)
static const uint8_t fire_frame_B[8] = {
    0x02,  //     *
    0x06,  //    **
    0x09,  //   *  *
    0x12,  //  *  *
    0x15,  //  * * *
    0x1D,  // *** **
    0x1F,  // *******
    0x0E   //  ***
};

static void lcd_enable_pulse(void) {
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_E_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_E_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void lcd_write_nibble(uint8_t nibble) {
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D4_PIN, (nibble >> 0) & 0x01);
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D5_PIN, (nibble >> 1) & 0x01);
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D6_PIN, (nibble >> 2) & 0x01);
    HAL_GPIO_WritePin(LCD_DATA_PORT, LCD_D7_PIN, (nibble >> 3) & 0x01);
    lcd_enable_pulse();
}

void lcd_send_cmd(char cmd) {
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS_PIN, GPIO_PIN_RESET); // RS = 0 for Command
    lcd_write_nibble((cmd >> 4) & 0x0F);                         // Send Upper 4 bits
    lcd_write_nibble(cmd & 0x0F);                                // Send Lower 4 bits
}

void lcd_send_data(char data) {
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS_PIN, GPIO_PIN_SET);   // RS = 1 for Data
    lcd_write_nibble((data >> 4) & 0x0F);                        // Send Upper 4 bits
    lcd_write_nibble(data & 0x0F);                               // Send Lower 4 bits
}

/**
 * @brief Programs a custom character pattern into CGRAM
 * @param loc Location index (0-7)
 * @param char_map Pointer to 8-byte character pixel map
 */
void lcd_create_custom_char(uint8_t loc, const uint8_t *char_map) {
    if (loc < 8) {
        // CGRAM address starts at 0x40. Each character takes 8 bytes.
        lcd_send_cmd(0x40 + (loc * 8));
        for (int i = 0; i < 8; i++) {
            lcd_send_data(char_map[i]);
        }
    }
}

void lcd_init(void) {
    // 4-bit mode initialization sequence
    HAL_Delay(50);
    lcd_write_nibble(0x03);
    HAL_Delay(5);
    lcd_write_nibble(0x03);
    HAL_Delay(1);
    lcd_write_nibble(0x03);
    HAL_Delay(10);

    lcd_write_nibble(0x02); // Set to 4-bit operation mode
    HAL_Delay(10);

    lcd_send_cmd(0x28);
        HAL_Delay(1);
        lcd_send_cmd(0x0C);
        HAL_Delay(1);
        lcd_send_cmd(0x06);
        HAL_Delay(1);
        lcd_send_cmd(0x01);
        HAL_Delay(2);

    // Write fire logo into CGRAM index 0
        lcd_create_custom_char(0, fire_frame_A);
        lcd_create_custom_char(1, fire_frame_B);

        lcd_send_cmd(0x80); // Clear DDRAM address pointer back to row 0, col 0
}

void lcd_send_string(char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    HAL_Delay(2);
}

void lcd_put_cur(int row, int col) {
    switch (row) {
        case 0:
            lcd_send_cmd(0x80 + col);
            break;
        case 1:
            lcd_send_cmd(0xC0 + col);
            break;
        case 2:
            lcd_send_cmd(0x94 + col);
            break;
        case 3:
            lcd_send_cmd(0xD4 + col);
            break;
    }
}
