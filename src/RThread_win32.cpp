#ifdef _WIN32
#include "RThread.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cpp11/R.hpp>
#include <cpp11.hpp>

#include <thread>
#include <chrono>
#include <deque>


#include "AsyncUtilsDebug.h"

namespace httpgd
{
    namespace async
    {
        namespace
        {
            

            const UINT HTTPGD_MESSAGE_ID = WM_USER + 201;
            threadsafe_queue<function_wrapper> work_queue;

            inline void process_tasks() {
                function_wrapper task;
                while (work_queue.try_pop(task))
                {
                    dbg_print("Do some work!...");
                    try {
                        R_ToplevelExec([](void *task_ptr){ (*(function_wrapper*)task_ptr).call(); }, &task);
                        //cpp11::unwind_protect([&](){ task.call(); });
                    } catch (const std::exception& e) {
                        dbg_print(e.what());
                    } catch (...) {
                        dbg_print("Unknown error");
                    }
                    dbg_print("...done!");
                }
                dbg_print("Nothing to do!");
            }

            LRESULT CALLBACK callbackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
            {
                switch (message)
                {
                case HTTPGD_MESSAGE_ID:
                    dbg_print("httpgd calls!");
                    process_tasks();
                    return 0;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }

            class WinMessageHandler
            {
            public:
                WinMessageHandler()
                {
                    static const TCHAR *class_name = TEXT("httpgd_window_class");
                    WNDCLASSEX wc = {};
                    wc.cbSize = sizeof(WNDCLASSEX);
                    wc.lpfnWndProc = callbackWndProc;
                    wc.hInstance = NULL;
                    wc.lpszClassName = class_name;

                    if (!RegisterClassEx(&wc))
                    {
                        REprintf("Failed to register window class\n");
                        return;
                    }

                    m_hwind = CreateWindowEx(0, class_name, TEXT("httpgd"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
                    if (!m_hwind)
                    {
                        REprintf("Failed to create message-only window\n");
                        return;
                    }

                    Rprintf("Initialized!\n");
                }

                ~WinMessageHandler()
                {
                    Rprintf("Destroyed!\n");
                    DestroyWindow(m_hwind);
                    Rprintf("done!\n");
                }

                void message()
                {
                    PostMessage(m_hwind, HTTPGD_MESSAGE_ID, 0, 0);
                }

            private:
                HWND m_hwind;
            } service_timer;

        }

        void r_thread_impl(function_wrapper &&task)
        {
            dbg_print("Submission.");
            work_queue.push(std::move(task));
            service_timer.message();
        }

    } // namespace service
} // namespace httpgd

#endif