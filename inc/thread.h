#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#ifdef WIN
#include <windows.h>
#define THREAD_FUNC_RET_TYPE long unsigned int
#define THREAD_FUNC_PROTO long unsigned int (*thread_function)(void *)
#define thread_id_t HANDLE
#endif

#ifdef LIN
#include <pthread.h>
#define THREAD_FUNC_RET_TYPE void*
#define THREAD_FUNC_PROTO void* (*thread_function)(void *)
#define thread_id_t pthread_t
#endif

bool TH_CreateThread (THREAD_FUNC_PROTO, void* args, thread_id_t* threadId);

void TH_DisposeThread (thread_id_t id);

void TH_WaitThreads (thread_id_t* ids, int nThreads);

#endif
