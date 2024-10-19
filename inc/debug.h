#ifndef DEBUG_H
#define DEBUG_H

#include "poker.h"

void D_WriteSideBySideBoards (struct card_t best[BOARD_SIZE], struct card_t worst[BOARD_SIZE], char* format, ...);

void D_WriteBoards (struct card_t (*boards)[BOARD_SIZE], int count, char* path);

#endif
