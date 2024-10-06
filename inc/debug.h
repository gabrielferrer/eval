#ifndef DEBUG_H
#define DEBUG_H

#include "poker.h"

void D_WriteSideBySideBoards (char* path, board_t best, board_t worst);

void D_WriteBoards (char* path, board_t* boards, int count);

#endif
