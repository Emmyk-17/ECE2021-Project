#ifndef READING_H
#define READING_H

typedef struct {
    double voltage;
    double current;
} Reading;

// Creates a new reading on the heap
Reading* reading_create(double voltage, double current);

// Frees a reading from memory
void reading_free(Reading* r);

#endif // READING_H
