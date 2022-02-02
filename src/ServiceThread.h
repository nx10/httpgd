#ifndef SERVICETHREAD_H
#define SERVICETHREAD_H

#include <thread>
#include <future>

#include "AsyncUtils.h"

namespace httpgd
{
    namespace async
    {
        const threadsafe_queue<function_wrapper> &get_work_queue();

        template <typename FunctionType>
        std::future<typename std::result_of<FunctionType()>::type>
        submit(FunctionType f)
        {
            typedef typename std::result_of<FunctionType()>::type
                result_type;
            std::packaged_task<result_type()> task(std::move(f));
            std::future<result_type> res(task.get_future());
            get_work_queue().push(std::move(task));
            return res;
        }

        void init();

    } // namespace async
} // namespace httpgd

#endif // SERVICETHREAD_H