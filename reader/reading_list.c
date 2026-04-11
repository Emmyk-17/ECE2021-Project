#include "reading_list.h"
#include <stdlib.h>

ReadingList* reading_list_create() {
    ReadingList* list = (ReadingList*)malloc(sizeof(ReadingList));
    if (!list) return NULL;
    list->head = list->tail = NULL;
    list->count = 0;
    return list;
}

void reading_list_free(ReadingList* list) {
    if (!list) return;
    ReadingNode* current = list->head;
    while (current) {
        ReadingNode* next = current->next;
        if (current->data)
            reading_free(current->data);
        free(current);
        current = next;
    }
    free(list);
}

void reading_list_add(ReadingList* list, double voltage, double current) {
    if (!list) return;

    Reading* r = reading_create(voltage, current);
    if (!r) return;

    ReadingNode* node = (ReadingNode*)malloc(sizeof(ReadingNode));
    if (!node) {
        reading_free(r);
        return;
    }

    node->data = r;
    node->next = NULL;

    if (!list->head) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->count++;
}

Reading* reading_list_average_and_clear(ReadingList* list) {
    if (!list || list->count == 0) return NULL;

    double sum_voltage = 0;
    double sum_current = 0;
    size_t cnt = 0;

    ReadingNode* current = list->head;
    while (current) {
        if (current->data) {
            sum_voltage += current->data->voltage;
            sum_current += current->data->current;
            cnt++;
        }
        current = current->next;
    }

    Reading* avg = reading_create(sum_voltage / cnt, sum_current / cnt);

    // Clear the list and free memory
    current = list->head;
    while (current) {
        ReadingNode* next = current->next;
        if (current->data) reading_free(current->data);
        free(current);
        current = next;
    }
    list->head = list->tail = NULL;
    list->count = 0;

    return avg;
}
