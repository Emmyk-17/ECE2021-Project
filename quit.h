#ifndef QUIT_H
#define QUIT_H

#include <termios.h>

typedef struct {
    struct termios term;
    int flags;
} TerminalState;

void enable_raw_mode(TerminalState *state);
void disable_raw_mode(TerminalState *state);

#endif
