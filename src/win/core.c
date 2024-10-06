#include <windows.h>
#include "core.h"

int GetCores ()
{
	SYSTEM_INFO info;

	GetSystemInfo (&info);

	return info.dwNumberOfProcessors;
}
