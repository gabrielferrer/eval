#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#ifdef WIN
#include <windows.h>
#define THREAD_FUNC_RET_TYPE long unsigned int
#define THREAD_FUNC_PROTO long unsigned int (*thread_function)(void *)
#define thread_id_t HANDLE
#define VALID_THREAD_ID(id) (id != NULL)
#endif

#ifdef LIN
#include <pthread.h>
#define THREAD_FUNC_RET_TYPE void
#define THREAD_FUNC_PROTO void (*thread_function)(void *)
#define thread_id_t int
#define VALID_THREAD_ID(id) (id != 0)
#endif

thread_id_t TH_CreateThread (THREAD_FUNC_PROTO, void* args);

void TH_CloseThread (thread_id_t id);

#endif
