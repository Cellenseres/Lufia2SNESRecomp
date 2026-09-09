#ifndef LUFIA2_LOG_H
#define LUFIA2_LOG_H

#include <stdio.h>

/* Progress and state tracing. Off in a shipped build: on a console stderr goes
 * to a file on the memory card, so a line per map change costs real I/O.
 *
 * Errors do not use this. A failed load, a rejected ROM or a refused presenter
 * is always reported. */
#ifdef LUFIA2_ENABLE_RUNTIME_LOG
#define LUFIA2_LOG(...) fprintf(stderr, __VA_ARGS__)
#define LUFIA2_LOG_FLUSH() fflush(stderr)
#else
#define LUFIA2_LOG(...) ((void)0)
#define LUFIA2_LOG_FLUSH() ((void)0)
#endif

#endif
