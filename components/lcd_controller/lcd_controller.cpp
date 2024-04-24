#include    "lcd_controller.h"

#include    <Arduino.h>
#include    <LiquidCrystal.h>

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void lcd_init() {
    lcd.begin(16, 2);
    lcd.noBlink();
    lcd.noCursor();
    delay(100);
}

void lcd_clear() {
    lcd.clear();
}

void lcd_printHome(const char* message) {
    lcd.clear();
    lcd.home();
    lcd.print(message);
}

void lcd_setCursor(uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
}

void lcd_printCustom(const char* message, uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
    lcd.print(message);
}