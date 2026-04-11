#ifndef LCD_METER_H
#define LCD_METER_H

#include <stdbool.h>

// Initialize the LCD and show startup message
// Returns true if success, false on failure
bool lcd_init_meter();

// Display voltage, current, and wattage
void lcd_show_stats(double voltage, double current, double wattage);

// Clear the LCD and show shutdown message
void lcd_power_down();

#endif // LCD_METER_H
