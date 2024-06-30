#ifndef DEBUG_H
#define DEBUG_H

#include "poker.h"

void D_write_boards(char* path, board_t best, board_t worst);

void D_write(char* path, board_t* boards, int count);

#endif
