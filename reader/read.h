#ifndef READ_H
#define READ_H

#include <stdint.h>

// ==========================
// Calibration / setup
// ==========================
void auto_calibrate(void);                     // Calibrate offsets only
void setVoltageGain(float gain);               // Manually set voltage gain

// ==========================
// RMS readings
// ==========================
float getVoltage(void);                        // Returns RMS voltage
float getCurrent(void);                        // Returns RMS current

#endif
