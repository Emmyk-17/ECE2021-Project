#include "lcd_meter.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <string.h>

#define I2C_BUS "/dev/i2c-1"
#define LCD_ADDR 0x27
#define LCD_BACKLIGHT 0x08
#define ENABLE 0x04
#define RS 0x01
#define LCD_WIDTH 20  // 4x20 LCD

static int lcd_file = -1;

// Line cache to avoid flicker
static char prev_line1[21] = "";
static char prev_line2[21] = "";
static char prev_line3[21] = "";
static char prev_line4[21] = "";

// ------------------ Low Level ------------------

static void lcd_write_byte(unsigned char data) {
    write(lcd_file, &data, 1);
}

static void lcd_strobe(unsigned char data) {
    lcd_write_byte(data | ENABLE | LCD_BACKLIGHT);
    usleep(500);
    lcd_write_byte((data & ~ENABLE) | LCD_BACKLIGHT);
    usleep(500);
}

static void lcd_write(unsigned char data, int mode) {
    unsigned char high = data & 0xF0;
    unsigned char low  = (data << 4) & 0xF0;
    lcd_strobe(high | (mode ? RS : 0));
    lcd_strobe(low  | (mode ? RS : 0));
}

static void lcd_clear() {
    lcd_write(0x01, 0);
    usleep(4000);
}

static void lcd_set_cursor(int row, int col) {
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > 3) row = 3;
    lcd_write(0x80 | (col + row_offsets[row]), 0);
}

static void lcd_write_string(const char *str) {
    while (*str) lcd_write(*str++, 1);
}

static void lcd_lowlevel_init() {
    lcd_write(0x33,0);
    lcd_write(0x32,0);
    lcd_write(0x28,0);
    lcd_write(0x0C,0);
    lcd_write(0x06,0);
    lcd_clear();
}

// Update line if changed
static void lcd_update_line(int row, char *new_line, char *prev_line) {
    if (strcmp(new_line, prev_line) != 0) {
        lcd_set_cursor(row, 0);
        lcd_write_string("                    "); // clear line
        lcd_set_cursor(row, 0);
        lcd_write_string(new_line);
        strncpy(prev_line, new_line, 20);
        prev_line[20] = '\0';
    }
}

// ------------------ Public Functions ------------------

bool lcd_init_meter() {
    lcd_file = open(I2C_BUS, O_RDWR);
    if (lcd_file < 0) return false;
    usleep(500000);
    if (ioctl(lcd_file, I2C_SLAVE, LCD_ADDR) < 0) {
        close(lcd_file);
        lcd_file = -1;
        return false;
    }
    lcd_lowlevel_init();

    lcd_set_cursor(0,0); lcd_write_string("ECE GROUP 2 METER");
    lcd_set_cursor(1,0); lcd_write_string("Initializing...");
    sleep(1);
    lcd_clear();

    return true;
}

// Main display
void lcd_show_stats(double voltage, double current, double wattage) {
    if (lcd_file < 0) return;

    char line1[21], line2[21], line3[21];
    char value_str[12];

    // Right-justify helper: format value to right-align within 10 chars
    // Total line: label 10 chars + value 10 chars = 20 chars

    // Line 1: Voltage
    snprintf(value_str, 12, "%7.2fV", voltage);
    snprintf(line1, 21, "%-10s%10s", "Voltage :", value_str);

    // Line 2: Current
    if (current < 1.0) snprintf(value_str, 12, "%7.0fmA", current*1000);
    else snprintf(value_str, 12, "%7.2fA", current);
    snprintf(line2, 21, "%-10s%10s", "Current :", value_str);

    // Line 3: Wattage
    snprintf(value_str, 12, "%7.2fW", wattage);
    snprintf(line3, 21, "%-10s%10s", "Wattage :", value_str);

    // Update lines
    lcd_update_line(0,line1,prev_line1);
    lcd_update_line(1,line2,prev_line2);
    lcd_update_line(2,line3,prev_line3);
}

// Power down
void lcd_power_down() {
    if (lcd_file < 0) return;
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_write_string("Powering down...");
    sleep(1);
    lcd_clear();
    close(lcd_file);
    lcd_file = -1;
}
