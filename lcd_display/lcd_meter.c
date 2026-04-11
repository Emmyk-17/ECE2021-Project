#include "lcd_meter.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
<<<<<<< HEAD
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

// Energy accumulator (Wh)
static double total_energy_wh = 0.0;

// ------------------ Low Level ------------------

static void lcd_write_byte(unsigned char data) {
    write(lcd_file, &data, 1);
}

=======

#define I2C_BUS "/dev/i2c-1"
#define LCD_ADDR 0x27      
#define LCD_BACKLIGHT 0x08
#define ENABLE 0x04
#define RS 0x01

static int lcd_file = -1;

// Helper: send a byte to LCD
static void lcd_write_byte(unsigned char data) {
    if (write(lcd_file, &data, 1) != 1) {
        perror("Failed to write byte to LCD");
    }
}

// Helper: toggle enable pin
>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
static void lcd_strobe(unsigned char data) {
    lcd_write_byte(data | ENABLE | LCD_BACKLIGHT);
    usleep(500);
    lcd_write_byte((data & ~ENABLE) | LCD_BACKLIGHT);
    usleep(500);
}

<<<<<<< HEAD
static void lcd_write(unsigned char data, int mode) {
    unsigned char high = data & 0xF0;
    unsigned char low  = (data << 4) & 0xF0;
=======
// Send command or data
static void lcd_write(unsigned char data, int mode) {
    unsigned char high = data & 0xF0;
    unsigned char low  = (data << 4) & 0xF0;

>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
    lcd_strobe(high | (mode ? RS : 0));
    lcd_strobe(low  | (mode ? RS : 0));
}

<<<<<<< HEAD
static void lcd_clear() {
    lcd_write(0x01, 0);
    usleep(4000);
}

=======
// Clear display
static void lcd_clear() {
    lcd_write(0x01, 0); // clear command
    usleep(4000);
}

// Set cursor position for 4-row LCD
>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
static void lcd_set_cursor(int row, int col) {
    int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > 3) row = 3;
    lcd_write(0x80 | (col + row_offsets[row]), 0);
}

<<<<<<< HEAD
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
=======
// Write a string
static void lcd_write_string(const char *str) {
    while (*str) {
        lcd_write(*str++, 1);
    }
}

// Low-level LCD init for 4-row
static void lcd_lowlevel_init() {
    // Safe 4-bit init sequence
    lcd_write(0x33,0);
    lcd_write(0x32,0);
    lcd_write(0x28,0); // 4-bit, 2 lines (works for 4-row)
    lcd_write(0x0C,0); // display on, cursor off
    lcd_write(0x06,0); // entry mode, cursor moves right
    lcd_clear();
}

// ------------------ Public Functions ------------------

// Initialize meter LCD
bool lcd_init_meter() {
    lcd_file = open(I2C_BUS, O_RDWR);
    if (lcd_file < 0) {
        perror("Error: Failed to open I2C bus");
        return false;
    }

    // Delay for LCD to power up
    usleep(500000); // 0.5 sec

    if (ioctl(lcd_file, I2C_SLAVE, LCD_ADDR) < 0) {
        perror("Error: Failed to acquire bus access or talk to slave");
>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
        close(lcd_file);
        lcd_file = -1;
        return false;
    }
<<<<<<< HEAD
    lcd_lowlevel_init();

    lcd_set_cursor(0,0); lcd_write_string("ECE GROUP 2 METER");
    lcd_set_cursor(1,0); lcd_write_string("Initializing...");
    sleep(1);
=======

    lcd_lowlevel_init();

    // Test text on all 4 rows
    lcd_set_cursor(0,0); lcd_write_string("ECE GROUP 2 METER");
    lcd_set_cursor(1,0); lcd_write_string("Status: Online");
    sleep(2);
>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
    lcd_clear();

    return true;
}

<<<<<<< HEAD
// Main display
void lcd_show_stats(double voltage, double current, double wattage) {
    if (lcd_file < 0) return;

    char line1[21], line2[21], line3[21], line4[21];
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

    // Line 4: Energy in Wh
    total_energy_wh += wattage / 3600.0;
    snprintf(value_str, 12, "%7.3fWh", total_energy_wh);
    snprintf(line4, 21, "%-10s%10s", "Energy  :", value_str);

    // Update lines
    lcd_update_line(0,line1,prev_line1);
    lcd_update_line(1,line2,prev_line2);
    lcd_update_line(2,line3,prev_line3);
    lcd_update_line(3,line4,prev_line4);
}

// Power down
void lcd_power_down() {
    if (lcd_file < 0) return;
=======
// Show voltage, current, wattage
void lcd_show_stats(double voltage, double current, double wattage) {
    if (lcd_file < 0) return;

    char line1[20], line2[20], line3[20];

    snprintf(line1, 20, "Voltage: %.2fV", voltage);
    snprintf(line2, 20, "Current: %.2fA", current);
    snprintf(line3, 20, "Wattage: %.2fW", wattage);

    lcd_clear();
    lcd_set_cursor(0,0); lcd_write_string(line1);
    lcd_set_cursor(1,0); lcd_write_string(line2);
    lcd_set_cursor(2,0); lcd_write_string(line3);
   
}
//Power down meter
void lcd_power_down() {
    if (lcd_file < 0) return;

>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_write_string("Powering down...");
    sleep(1);
    lcd_clear();
<<<<<<< HEAD
=======

>>>>>>> dd4b5c7ec49655ad7f32258ad4a4acbaaf1972f3
    close(lcd_file);
    lcd_file = -1;
}
