#ifndef HTTPGD_ASYNC_UTILS_DEBUG_H
#define HTTPGD_ASYNC_UTILS_DEBUG_H

#include <cpp11/R.hpp>
#include <thread>
#include <string>
#include <sstream>

namespace httpgd
{
    namespace async
    {
        template <typename T>
        inline void dbg_print(T message)
        {
            std::ostringstream oss;
            oss << "[Thread #" << std::this_thread::get_id() << "] " << message << std::endl;
            Rprintf(oss.str().c_str());
        }
    }
}

#endif 