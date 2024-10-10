#ifndef DEBUG_H
#define DEBUG_H

#include "poker.h"

void D_WriteSideBySideBoards (char* path, struct card_t best[BOARD_SIZE], struct card_t worst[BOARD_SIZE]);

void D_WriteBoards (char* path, struct card_t (*boards)[BOARD_SIZE], int count);

#endif
