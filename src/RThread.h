#ifndef SERVICETHREAD_H
#define SERVICETHREAD_H

#include <thread>
#include <future>

#include "AsyncUtils.h"

namespace httpgd
{
    namespace async
    {
        void ipc_open();
        void ipc_close();

        void r_thread_impl(function_wrapper &&f);

        template <typename FunctionType>
        std::future<typename std::result_of<FunctionType()>::type>
        r_thread(FunctionType f)
        {
            typedef typename std::result_of<FunctionType()>::type
                result_type;
            std::packaged_task<result_type()> task(std::move(f));
            std::future<result_type> res(task.get_future());
            r_thread_impl(std::move(task));
            return res;
        }

    } // namespace async
} // namespace httpgd

#endif // SERVICETHREAD_H