#include "reading.h"
#include <stdlib.h>

Reading* reading_create(double voltage, double current) {
    Reading* r = (Reading*)malloc(sizeof(Reading));
    if (!r) return NULL;
    r->voltage = voltage;
    r->current = current;
    return r;
}

void reading_free(Reading* r) {
    if (r) free(r);
}
