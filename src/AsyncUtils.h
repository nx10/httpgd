#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

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
            void push(T new_value)
            {
                std::lock_guard<std::mutex> lk(mut);
                data_queue.push(new_value);
                data_cond.notify_one();
            }
            void wait_and_pop(T &value)
            {
                std::unique_lock<std::mutex> lk(mut);
                data_cond.wait(lk, [this]
                               { return !data_queue.empty(); });
                value = data_queue.front();
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
                value = data_queue.front();
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
            template <typename F>
            function_wrapper(F &&f) : impl(new impl_type<F>(std::move(f)))
            {
            }
            void operator()() { impl->call(); }
            function_wrapper() = default;
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
}

#endif // SAFEQUEUE_H