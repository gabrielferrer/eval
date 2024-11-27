#include "thread.h"

bool TH_CreateThread (THREAD_FUNC_PROTO, void* args, thread_id_t* threadId)
{
	*threadId = CreateThread (NULL, 0, thread_function, args, 0, NULL);
	
	return *threadId != NULL;
}

void TH_DisposeThread (thread_id_t id)
{
	CloseHandle (id);
}

void TH_WaitThreads (thread_id_t* ids, int nThreads)
{
	WaitForMultipleObjects(nThreads, ids, TRUE, INFINITE);
}
