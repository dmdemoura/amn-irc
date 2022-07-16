#include "application.h"

#include <signal.h>
#include <time.h>

static volatile sig_atomic_t ShouldShutdown = 0;

void Application_StartShutdown()
{
	ShouldShutdown = 1;
}

bool Application_ShouldShutdown()
{
	return ShouldShutdown == 1;
}
