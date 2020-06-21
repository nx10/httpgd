#ifndef HTTPGD_LIB_CPPHTTPLIB_H
#define HTTPGD_LIB_CPPHTTPLIB_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
// Taken from http://tolstoy.newcastle.edu.au/R/e2/devel/06/11/1242.html
// Undefine the Realloc macro, which is defined by both R and by Windows stuff
#undef Realloc
// Also need to undefine the Free macro
#undef Free
#include <winsock2.h>
#endif // _WIN32
#ifdef CPPHTTPLIB_USE_POLL
#undef CPPHTTPLIB_USE_POLL
#endif
#include "cpphttplib/httplib.h"

#endif