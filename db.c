#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libpq-fe.h>

static PGconn *conn = NULL;

// Neon DB connection string baked in
static const char *connString =
    "postgresql://neondb_owner:npg_EWHs9FS6OazR@ep-withered-queen-ajwazud6-pooler.c-3.us-east-2.aws.neon.tech/"
    "neondb?sslmode=require&channel_binding=require";

// Internal helper to check connection
static bool check_conn() {
    if (!conn) {
        fprintf(stderr, "DB connection not initialized.\n");
        return false;
    }
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "DB connection bad: %s\n", PQerrorMessage(conn));
        return false;
    }
    return true;
}

// Initialize DB connection
bool db_init(void) {
    conn = PQconnectdb(connString);
    if (!check_conn()) {
        PQfinish(conn);
        conn = NULL;
        return false;
    }
    return true;
}

// Insert a reading into the database with millisecond-precision timestamp
bool db_insert_reading(const Reading* reading) {
    if (!check_conn() || !reading) return false;

    // Get current time with nanosecond precision
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // seconds + nanoseconds

    struct tm tm_now;
    gmtime_r(&ts.tv_sec, &tm_now); // UTC

    char timestamp_sec[32];
    strftime(timestamp_sec, sizeof(timestamp_sec), "%Y-%m-%d %H:%M:%S", &tm_now);

    int millis = ts.tv_nsec / 1000000; // convert nanoseconds to milliseconds

    char timestamp_str[64];
    snprintf(timestamp_str, sizeof(timestamp_str), "%s.%03d+0000", timestamp_sec, millis);

    // Build query
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO public.readings (timestamp, voltage, current) "
             "VALUES ('%s', %f, %f);",
             timestamp_str, reading->voltage, reading->current);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Insert failed: %s\nQuery: %s\n", PQerrorMessage(conn), query);
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

// Get the latest reading from the database
Reading* db_get_latest_reading(void) {
    if (!check_conn()) return NULL;

    const char *query =
        "SELECT voltage, current "
        "FROM public.readings "
        "ORDER BY timestamp DESC "
        "LIMIT 1;";

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return NULL;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return NULL; // no rows
    }

    double voltage = atof(PQgetvalue(res, 0, 0));
    double current = atof(PQgetvalue(res, 0, 1));

    Reading* r = reading_create(voltage, current);

    PQclear(res);
    return r;
}

// Close the DB connection
void db_close(void) {
    if (conn) {
        PQfinish(conn);
        conn = NULL;
    }
}
