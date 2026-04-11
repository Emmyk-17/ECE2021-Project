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
    } while (!success && tries>0);
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

    // Variables for delayed  averaging
    struct timespec start_time, now;
    double elapsed_ms;

    clock_gettime(CLOCK_MONOTONIC, &start_time); // initialize start time before loop

    // main loop
    while(true)
    {
        // read current and voltage from ADC
        voltage = getVoltage();
        current = getCurrent();

        // add data to linked list
        reading_list_add(list, voltage, current);

        // check if delay passed
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_ms = (now.tv_sec - start_time.tv_sec) * 1000.0
                   + (now.tv_nsec - start_time.tv_nsec) / 1000000.0;

        // average linked list after delay 
        if (elapsed_ms >= DELAY) {
            average = reading_list_average_and_clear(list);

            // save averaged reading to database 
            if (average) {
                success = db_insert_reading(average);
		if (!success)
		{
		    printf("Error inserting value\n");
		}
                reading_free(average);
                average = NULL;
            }

            // reset start time for next interval
            start_time = now;
        }

        // if q is pressed exit loop 
        c = getchar();
        if (c != EOF) {
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
