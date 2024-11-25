#include <thread.h>

bool TH_CreateThread (THREAD_FUNC_PROTO, void* args, thread_id_t* threadId)
{
	return pthread_create (threadId, NULL, thread_function, args) == 0;
}

void TH_DisposeThread (thread_id_t id)
{
}

void TH_WaitThreads (thread_id_t* ids, int nThreads)
{
	for (int i = 0; i < nThreads; i++)
	{
		pthread_join (ids[i], NULL);
	}
}

