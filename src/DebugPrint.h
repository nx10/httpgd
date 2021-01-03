#pragma once
#ifndef HTTPGD_DEBUG_DEVICE
#define debug_print(...)
#else
#define debug_print(fmt, ...) Rprintf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);
#endif