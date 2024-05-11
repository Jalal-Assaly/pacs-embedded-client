#ifndef LCD_CONTROLLER_H
#define LCD_CONTROLLER_H

#include    <stdint.h>

#define     LCD_RS        (33)
#define     LCD_E         (25)

#define     LCD_D4        (26)
#define     LCD_D5        (27)
#define     LCD_D6        (14)
#define     LCD_D7        (13)

void lcd_init();
void lcd_clear();
void lcd_printHome(const char* message);
void lcd_setCursor(uint8_t col, uint8_t row);
void lcd_printCustom(const char* message, uint8_t col, uint8_t row);

#endif // LCD_CONTROLLER_H
