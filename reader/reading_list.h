#ifndef READING_LIST_H
#define READING_LIST_H

#include "../reading.h"
#include <stddef.h>

typedef struct ReadingNode {
    Reading* data;
    struct ReadingNode* next;
} ReadingNode;

typedef struct {
    ReadingNode* head;
    ReadingNode* tail;
    size_t count;
} ReadingList;

// Initializes an empty list
ReadingList* reading_list_create();

// Frees all nodes and optionally the readings themselves
void reading_list_free(ReadingList* list);

// Adds a reading to the list (allocates reading internally)
void reading_list_add(ReadingList* list, double voltage, double current);

// Computes average voltage and current, returns a new reading on heap
// Clears the list after computing average
Reading* reading_list_average_and_clear(ReadingList* list);

#endif // READING_LIST_H
