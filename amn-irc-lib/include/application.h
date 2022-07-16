#ifndef AMN_APPLICATION_H
#define AMN_APPLICATION_H

#include <stdbool.h>
#include <time.h>

#include <sys/time.h>

void Application_StartShutdown();

bool Application_ShouldShutdown();

#endif // AMN_APPLICATION_H
