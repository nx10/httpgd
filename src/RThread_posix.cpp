#ifndef _WIN32
#include "RThread.h"

namespace httpgd {
    namespace async {
        void r_thread_impl(function_wrapper &&task)
        {
            dbg_print("Submission.");
        }
    }
}



#endif