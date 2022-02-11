#ifndef _WIN32
#include "RThread.h"

#include <cpp11/R.hpp>
#include <cpp11/protect.hpp>
#include <R_ext/eventloop.h> // for addInputHandler()

#include <thread>
#include <unistd.h>

#include "AsyncUtilsDebug.h"

namespace httpgd {
    namespace async {
        namespace {
            const int HTTPGD_ACTIVITY_ID = 501;
            const size_t HTTPGD_PIPE_BUFFER_SIZE = 32;
            threadsafe_queue<function_wrapper> work_queue;

            inline void process_tasks() {
                function_wrapper task;
                while (work_queue.try_pop(task))
                {
                    dbg_print("Do some work!...");
                    try {
                        //R_ToplevelExec([](void *task_ptr){ (*(function_wrapper*)task_ptr).call(); }, &task);
                        cpp11::unwind_protect([&](){ task.call(); });
                    } catch (const std::exception& e) {
                        dbg_print(e.what());
                    } catch (...) {
                        dbg_print("Unknown error");
                    }
                    dbg_print("...done!");
                }
                dbg_print("Nothing to do!");
            }

            void input_handler(void* userData);

            class PosixMessageHandler
            {
            public:
                PosixMessageHandler()
                {
                    dbg_print("Message handler initialized.");

                    
                    if (pipe(m_fd) == -1) {
                        dbg_print("Could not create pipe");
                    }

                    m_input_handle = addInputHandler(R_InputHandlers, m_fd[0], input_handler, HTTPGD_ACTIVITY_ID);
                }

                ~PosixMessageHandler()
                {
                    dbg_print("Message handler destroying...");
                    
                    removeInputHandler(&R_InputHandlers, m_input_handle);
                    close(m_fd[0]);
                    close(m_fd[1]);

                    dbg_print("...message handler done!");
                }

                void message()
                {
                    if (write(m_fd[1], "h", 1) == -1) {
                        dbg_print("Could not write to pipe");
                    }
                }

                void empty_pipe() {
                    if (read(m_fd[0], m_buf, HTTPGD_PIPE_BUFFER_SIZE) == -1) {
                        dbg_print("Could not read from pipe");
                    }
                }

            private:
                int m_fd[2];
                char m_buf[HTTPGD_PIPE_BUFFER_SIZE];
                InputHandler* m_input_handle;
                
            } message_handler;

            void input_handler(void* userData) {
                message_handler.empty_pipe();

                dbg_print("input handler");
                process_tasks();
            }
        }

        void r_thread_impl(function_wrapper &&task)
        {
            dbg_print("Submission.");
            work_queue.push(std::move(task));
            message_handler.message();
        }
    }
}



#endif