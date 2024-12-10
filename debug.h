/*
 *  Queueing/Scheduling Lab
 *
 *  Copyright (c) 2022 Peter Dinda, Branden Ghena
 *
 *  Original Queuesim tool is Copyright (c) 2005 Peter Dinda
 */

#ifndef _debug
#define _debug

#include <stdio.h>


// enable your debugging options here
// if debug is on for a support module, that module's DEBUG() statements will print
#define DEBUG_EVENT        1
#define DEBUG_EVENT_QUEUE  1
#define DEBUG_JOB          1
#define DEBUG_JOB_QUEUE    1
#define DEBUG_CONTEXT      1

// the following are the macros for output
// in case you want to log elsewhere
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: " fmt, ##args);
#define ERROR_PRINT(fmt, args...) fprintf(stderr, "ERROR (%s:%d): " fmt, __FILE__, __LINE__, ##args);
#define INFO_PRINT(fmt, args...) fprintf(stderr, fmt, ##args);

#endif

