#ifndef _WIN32
#include "RThread.h"

#include <cpp11/R.hpp>
#include <cpp11/protect.hpp>
#include <R_ext/eventloop.h> // for addInputHandler()

#include <thread>
#include <unistd.h>

namespace httpgd {
    namespace async {
        namespace {
            const int HTTPGD_ACTIVITY_ID = 501;
            const size_t HTTPGD_PIPE_BUFFER_SIZE = 32;
            threadsafe_queue<function_wrapper> work_queue;
            int message_fd[2];
            char message_buf[HTTPGD_PIPE_BUFFER_SIZE];
            InputHandler* message_input_handle;

            inline void r_print_error(const char *message) {
                REprintf("Error (httpgd IPC): %s\n", message);
            }

            inline void process_tasks() {
                function_wrapper task;
                while (work_queue.try_pop(task))
                {
                    task.call();
                }
            }

            inline void notify_work()
            {
                if (write(message_fd[1], "h", 1) == -1) {
                    r_print_error("Could not write to pipe");
                }
            }

            inline void empty_pipe() {
                if (read(message_fd[0], message_buf, HTTPGD_PIPE_BUFFER_SIZE) == -1) {
                    r_print_error("Could not read from pipe");
                }
            }

            void input_handler(void* userData) {
                empty_pipe();
                process_tasks();
            }
        }
        
        void ipc_open() 
        {   
            if (pipe(message_fd) == -1) {
                r_print_error("Could not create pipe");
            }

            message_input_handle = addInputHandler(R_InputHandlers, message_fd[0], input_handler, HTTPGD_ACTIVITY_ID);
        }
        
        void ipc_close() 
        {
            removeInputHandler(&R_InputHandlers, message_input_handle);
            close(message_fd[0]);
            close(message_fd[1]);
        }

        void r_thread_impl(function_wrapper &&task)
        {
            work_queue.push(std::move(task));
            notify_work();
        }
    }
}



#endif