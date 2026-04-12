#ifndef DB_H
#define DB_H

#include <stdbool.h>
#include "reading.h"

// Initializes the database connection
bool db_init();

// Inserts a reading into the database with timestamp generated in C
bool db_insert_reading(const Reading* reading);

// Inserts an alert into the db
void db_insert_alert(const char *message);

// Retrieves the latest reading from the database
Reading* db_get_latest_reading();

// Closes the database connection
void db_close();

#endif // DB_H
