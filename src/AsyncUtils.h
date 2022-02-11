#ifndef HTTPGD_ASYNC_UTILS_H
#define HTTPGD_ASYNC_UTILS_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace httpgd
{

    namespace async
    {

        template <typename T>
        class threadsafe_queue
        {
        private:
            mutable std::mutex mut;
            std::queue<T> data_queue;
            std::condition_variable data_cond;

        public:
            threadsafe_queue()
            {
            }
            threadsafe_queue(threadsafe_queue const &other)
            {
                std::lock_guard<std::mutex> lk(other.mut);
                data_queue = other.data_queue;
            }
            void push(T &&new_value)
            {
                std::lock_guard<std::mutex> lk(mut);
                data_queue.push(std::move(new_value));
                data_cond.notify_one();
            }
            void wait_and_pop(T &value)
            {
                std::unique_lock<std::mutex> lk(mut);
                data_cond.wait(lk, [this]
                               { return !data_queue.empty(); });
                value = std::move(data_queue.front());
                data_queue.pop();
            }
            std::shared_ptr<T> wait_and_pop()
            {
                std::unique_lock<std::mutex> lk(mut);
                data_cond.wait(lk, [this]
                               { return !data_queue.empty(); });
                std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
                data_queue.pop();
                return res;
            }
            bool try_pop(T &value)
            {
                std::lock_guard<std::mutex> lk(mut);
                if (data_queue.empty())
                    return false;
                value = std::move(data_queue.front());
                data_queue.pop();
                return true;
            }
            std::shared_ptr<T> try_pop()
            {
                std::lock_guard<std::mutex> lk(mut);
                if (data_queue.empty())
                    return std::shared_ptr<T>();
                std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
                data_queue.pop();
                return res;
            }
            bool empty() const
            {
                std::lock_guard<std::mutex> lk(mut);
                return data_queue.empty();
            }
        };

        class function_wrapper
        {
            struct impl_base
            {
                virtual void call() = 0;
                virtual ~impl_base() {}
            };
            std::unique_ptr<impl_base> impl;
            template <typename F>
            struct impl_type : impl_base
            {
                F f;
                impl_type(F &&f_) : f(std::move(f_)) {}
                void call() { f(); }
            };

        public:
            function_wrapper() = default;

            template <typename F>
            function_wrapper(F &&f) : impl(new impl_type<F>(std::move(f)))
            {
            }

            void call() { impl->call(); }

            function_wrapper(function_wrapper &&other) : impl(std::move(other.impl))
            {
            }

            function_wrapper &operator=(function_wrapper &&other)
            {
                impl = std::move(other.impl);
                return *this;
            }

            function_wrapper(const function_wrapper &) = delete;
            function_wrapper(function_wrapper &) = delete;
            function_wrapper &operator=(const function_wrapper &) = delete;
        };
    }

    /*inline void print_thread_id()
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id() << std::endl;
        REprintf("Thread ID: %s", oss.str().c_str());
    }

    template<typename T>
    inline void print_thread_id(T message)
    {
        std::ostringstream oss;
        oss << "Thread #" << std::this_thread::get_id() << " : " << message << std::endl;
        REprintf(oss.str().c_str());
    }*/

}

#endif // HTTPGD_ASYNC_UTILS_H