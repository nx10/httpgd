#ifdef _WIN32
#include "RThread.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cpp11/R.hpp>
#include <cpp11/protect.hpp>
#include <thread>

#include "AsyncUtilsDebug.h"

namespace httpgd
{
    namespace async
    {
        namespace
        {
            const auto *HTTPGD_WINDOW_CLASS_NAME = TEXT("httpgd_window_class");
            const UINT HTTPGD_MESSAGE_ID = WM_USER + 201;
            threadsafe_queue<function_wrapper> work_queue;
            bool ipc_initialized{false};
            HWND message_hwind;

            inline void process_tasks() {
                function_wrapper task;
                while (work_queue.try_pop(task))
                {
                    try {
                        // I am not sure which is better:
                        //R_ToplevelExec([](void *task_ptr){ (*(function_wrapper*)task_ptr).call(); }, &task);
                        cpp11::unwind_protect([&](){ task.call(); });
                    } catch (const std::exception& e) {
                        REprintf("httpgd error: '%s'\n", e.what());
                    } catch (...) {
                        REprintf("httpgd unknown error.");
                    }
                }
            }

            LRESULT CALLBACK callbackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
            {
                switch (message)
                {
                case HTTPGD_MESSAGE_ID:
                    process_tasks();
                    return 0;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }

            inline bool window_class_exists() {
                WNDCLASSEX wc{};
                return GetClassInfoEx(GetModuleHandle(NULL), HTTPGD_WINDOW_CLASS_NAME, &wc) != 0;
            }

            inline bool register_window_class() {
                WNDCLASSEX wc = {};
                wc.cbSize = sizeof(WNDCLASSEX);
                wc.lpfnWndProc = callbackWndProc;
                wc.hInstance = GetModuleHandle(NULL);
                wc.lpszClassName = HTTPGD_WINDOW_CLASS_NAME;
                return RegisterClassEx(&wc) != 0;
            }
            
            inline HWND create_message_window() {
                return CreateWindowEx(0, HTTPGD_WINDOW_CLASS_NAME, TEXT("httpgd"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
            }

            inline void notify()
            {
                PostMessage(message_hwind, HTTPGD_MESSAGE_ID, 0, 0);
            }
        }

        void ipc_open() 
        {
            if (ipc_initialized) return;

            if (!window_class_exists()) { 
                // If there is already another version of httpgd loaded. (E.g. with pkgload::load_all,
                // which, unfortunately does not call .onUnload),
                // we have to re-use the (unique) window class, but should not reuse the message window. 
                // Otherwise we could not destroy the window while it is in use.
                // As the callback lives inside the window class, messages be received only once.
                
                //message_hwind = FindWindowEx(NULL, NULL, HTTPGD_WINDOW_CLASS_NAME, TEXT("httpgd"));

                if (!register_window_class())
                {
                    REprintf("httpgd: Failed to register window class.\n");
                }
            }

            message_hwind = create_message_window();
            if (!message_hwind)
            {
                cpp11::stop("httpgd: Failed to create message window.");
                return;
            }
            ipc_initialized = true;
        }
        
        void ipc_close() 
        {
            if (!ipc_initialized) return;

            if (DestroyWindow(message_hwind) == 0) {
                REprintf("httpgd: Failed to destroy message window.\n");
            }

            // We can not be sure if there is another dll instance loaded with pkgload::load_all
            // so we cannot unregister the window class.

            //if (UnregisterClass(HTTPGD_WINDOW_CLASS_NAME, NULL) == 0) {
            //    REprintf("httpgd: Failed to unregister window class.\n");
            //}

            ipc_initialized = false;
        }

        void r_thread_impl(function_wrapper &&task)
        {
            work_queue.push(std::move(task));
            notify();
        }

    } // namespace service
} // namespace httpgd

#endif