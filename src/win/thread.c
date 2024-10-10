#include "thread.h"

thread_id_t TH_CreateThread (THREAD_FUNC_PROTO, void* args)
{
	return CreateThread (NULL, 0, thread_function, args, 0, NULL);
}

void TH_DisposeThread (thread_id_t id)
{
	CloseHandle (id);
}

void TH_WaitThreads (thread_id_t* ids, int nThreads)
{
	WaitForMultipleObjects(nThreads, ids, TRUE, INFINITE);
}
