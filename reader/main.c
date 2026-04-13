#include "read.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "../quit.h"
#include "../db.h"
#include "../reading.h"
#include "reading_list.h"
#include <time.h>

#define DELAY 1000.0

// alert settings
#define POWER_BUF_SIZE 8
#define ALERT_COOLDOWN_MS 5000

int main()
{
    // setup wiring pi
    wiringPiSetup();
    wiringPiSPISetup(0, 1000000);

    // Initialize Database Connection
    bool success = false;
    int tries = 3;
    do
    {
        success = db_init();
        tries--;
    } while (!success && tries > 0);

    if (!success)
    {
        printf("Error Initializing DB\n");
        return 0;
    }

    // calibrate sensors
    setVoltageGain(1720.0f);
    auto_calibrate();

    // allow input while looping
    TerminalState state;
    enable_raw_mode(&state);
    char c;
    printf("Press 'q' to stop program\n");

    double voltage;
    double current;
    ReadingList* list = reading_list_create();
    Reading* average = NULL;

    // Variables for delayed averaging
    struct timespec start_time, now, last_alert_time;
    double elapsed_ms;
    double cooldown_ms;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    clock_gettime(CLOCK_MONOTONIC, &last_alert_time);

    // power buffer (now for averaged values, not raw)
    double power_buf[POWER_BUF_SIZE] = {0};
    int buf_index = 0;
    int buf_filled = 0;

    // main loop
    while (true)
    {
        // read current and voltage from ADC
        voltage = getVoltage();
        current = getCurrent();

        // add data to linked list
        reading_list_add(list, voltage, current);

        // check if delay passed
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_ms =
            (now.tv_sec - start_time.tv_sec) * 1000.0 +
            (now.tv_nsec - start_time.tv_nsec) / 1000000.0;

        // average linked list after delay
        if (elapsed_ms >= DELAY)
        {
            average = reading_list_average_and_clear(list);

            if (average)
            {
                success = db_insert_reading(average);
                if (!success)
                {
                    printf("Error inserting value\n");
                }

                // ---------------- ALERTS NOW USE AVERAGED VALUES ----------------

                double avg_voltage = average->voltage;
                double avg_current = average->current;
                double power = avg_voltage * avg_current;

                // update buffer with averaged power
                power_buf[buf_index] = power;
                buf_index = (buf_index + 1) % POWER_BUF_SIZE;
                if (buf_filled < POWER_BUF_SIZE) buf_filled++;

                // cooldown check
                clock_gettime(CLOCK_MONOTONIC, &now);
                cooldown_ms =
                    (now.tv_sec - last_alert_time.tv_sec) * 1000.0 +
                    (now.tv_nsec - last_alert_time.tv_nsec) / 1000000.0;

                bool can_alert = cooldown_ms > ALERT_COOLDOWN_MS;

                // ---------------- SPIKE DETECTION ----------------
                if (buf_filled == POWER_BUF_SIZE && can_alert)
                {
                    double sum = 0;
                    for (int i = 0; i < POWER_BUF_SIZE; i++)
                        sum += power_buf[i];

                    double avg = sum / POWER_BUF_SIZE;

                    if (avg > 0 && power > avg * 1.5)
                    {
                        char msg[128];
                        snprintf(msg, sizeof(msg),
                                 "Power spike detected (%.2f W)", power);

                        db_insert_alert(msg);
                        clock_gettime(CLOCK_MONOTONIC, &last_alert_time);
                    }
                }

                // ---------------- THRESHOLDS ----------------
                if (can_alert)
                {
                    if (power > 800.0)
                    {
                        char msg[128];
                        snprintf(msg, sizeof(msg),
                                 "Power threshold exceeded (%.2f W)", power);

                        db_insert_alert(msg);
                        clock_gettime(CLOCK_MONOTONIC, &last_alert_time);
                    }
                    else if (avg_voltage > 260.0)
                    {
                        char msg[128];
                        snprintf(msg, sizeof(msg),
                                 "Voltage anomaly detected (%.2f V)", avg_voltage);

                        db_insert_alert(msg);
                        clock_gettime(CLOCK_MONOTONIC, &last_alert_time);
                    }
                    else if (avg_current > 15.0)
                    {
                        char msg[128];
                        snprintf(msg, sizeof(msg),
                                 "Current spike detected (%.2f A)", avg_current);

                        db_insert_alert(msg);
                        clock_gettime(CLOCK_MONOTONIC, &last_alert_time);
                    }
                }

                reading_free(average);
                average = NULL;
            }

            // reset start time for next interval
            start_time = now;
        }

        // if q is pressed exit loop
        c = getchar();
        if (c != EOF)
        {
            if (c == 'q')
            {
                disable_raw_mode(&state);
                printf("\nExited cleanly.\n");
                break;
            }
        }
    }

    db_close();
    return 0;
}
