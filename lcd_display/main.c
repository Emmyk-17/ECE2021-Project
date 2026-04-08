#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../reading.h"
#include "../db.h"
#include "lcd_meter.h"
#include "../quit.h"

int main()
{
    // initialize display
    bool displayError = lcd_init_meter();
    if (!displayError)
    {
        printf("ERROR INITIALIZING DISPLAY!!!\n");
        return 0;
    }

    // Initialize Database Connection with 3 tries
    bool success = false;
    int tries = 3;
    do
    {
        success = db_init();
        tries--;
    } while (!success && tries > 0);

    if (!success)
    {
        printf("ERROR INITIALIZING DATABASE CONNECTION!!!\n");
        lcd_power_down();
        return 0;
    }

    // allow input while looping
    TerminalState state;
    enable_raw_mode(&state);
    char c;
    printf("Press 'q' to stop program\n");

    while(true)
    {
        // get latest reading from database
        Reading* latest = db_get_latest_reading();
        double voltage = 0;
        double current = 0;
        double wattage = 0;

        if (latest)
        {
            voltage = latest->voltage;
            current = latest->current;
            wattage = voltage * current;

            reading_free(latest); // free the Reading struct
        }

        // display on lcd display
        lcd_show_stats(voltage, current, wattage);

        // if q is pressed exit loop 
        c = getchar();
        if (c != EOF) {
            if (c == 'q') 
            {
                disable_raw_mode(&state);
                lcd_power_down();
                db_close();
                printf("\nExited cleanly.\n");
                break;
            }
        }

        // update every 1000 ms
        usleep(1000000);
    }

    // clean up db connection
    db_close();

    return 0;
}
