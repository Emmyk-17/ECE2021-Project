#include "quit.h"
#include <unistd.h>
#include <fcntl.h>

void enable_raw_mode(TerminalState *state) {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &state->term);
    raw = state->term;

    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    state->flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, state->flags | O_NONBLOCK);
}

void disable_raw_mode(TerminalState *state) {
    tcsetattr(STDIN_FILENO, TCSANOW, &state->term);
    fcntl(STDIN_FILENO, F_SETFL, state->flags);
}
