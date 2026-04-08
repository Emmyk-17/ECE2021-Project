#include "read.h"
#include <wiringPiSPI.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#define SAMPLE_COUNT 500
#define SAMPLE_DELAY_US 500
#define VREF 5.0f
#define ADC_MAX 1023.0f

static float currentSensitivity = 0.100f; // V/A
static float currentScale       = 2.0f;   // current sensor scaling
static float voltageDivider     = 2.0f;   // voltage divider halves the voltage

static float voltageOffset = 0.0f;
static float currentOffset = 0.0f;
static float voltageGain   = 1.0f; // manually set

static uint16_t read_adc(uint8_t channel) {
    uint8_t buffer[3];
    buffer[0] = 1;
    buffer[1] = (8 + channel) << 4;
    buffer[2] = 0;
    wiringPiSPIDataRW(0, buffer, 3);
    return ((buffer[1] & 3) << 8) | buffer[2];
}

// ==========================
// Set voltage gain manually
// ==========================
void setVoltageGain(float gain) {
    voltageGain = gain;
    printf("Voltage gain set to %.4f\n", voltageGain);
}

// ==========================
// Auto-calibration: offsets only
// ==========================
void auto_calibrate(void) {
    float vSum = 0.0f;
    float iSum = 0.0f;

    // Warm-up readings and average
    for (int n = 0; n < SAMPLE_COUNT; n++) {
        float v = (read_adc(1) / ADC_MAX) * VREF / voltageDivider;
        float c = (read_adc(0) / ADC_MAX) * VREF * currentScale;

        vSum += v;
        iSum += c;

        usleep(SAMPLE_DELAY_US);
    }

    voltageOffset = vSum / SAMPLE_COUNT;
    currentOffset = iSum / SAMPLE_COUNT;

    printf("Calibration done: VoltageOffset=%.4f V, CurrentOffset=%.4f V\n",
           voltageOffset, currentOffset);
}

// ==========================
// RMS voltage
// ==========================
float getVoltage(void) {
    float sumSq = 0.0f;

    for (uint16_t n = 0; n < SAMPLE_COUNT; n++) {
        float v = (read_adc(1) / ADC_MAX) * VREF / voltageDivider;
        float centered = v - voltageOffset;
        sumSq += centered * centered;
        usleep(SAMPLE_DELAY_US);
    }

    float rms =  sqrtf(sumSq / SAMPLE_COUNT) * voltageGain;
    rms = rms - 9;
    if (rms < 0)
    {
        rms = 0;
    }
    return rms;
}

// ==========================
// RMS current
// ==========================
float getCurrent(void) {
    float sumSq = 0.0f;

    for (uint16_t n = 0; n < SAMPLE_COUNT; n++) {
        float c = (read_adc(0) / ADC_MAX) * VREF * currentScale;
        float centered = c - currentOffset;
        sumSq += centered * centered;
        usleep(SAMPLE_DELAY_US);
    }

    float rms =  sqrtf(sumSq / SAMPLE_COUNT) / currentSensitivity;
    rms = rms - 0.16;
    if (rms < 0)
    {
        rms = 0;
    }
    return rms;
}
