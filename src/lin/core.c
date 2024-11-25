#include <unistd.h>
#include <core.h>

int GetCores ()
{
	return sysconf (_SC_NPROCESSORS_ONLN);
}

