#ifdef _WIN32
#include "ServiceThread.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cpp11/R.hpp>

#include <string>
#include <thread>
#include <sstream>
#include <chrono>

namespace httpgd
{
    namespace async
    {
        static const UINT HTTPGD_MESSAGE_ID =  WM_USER + 201;

        static void print_thread_id()
        {
            std::ostringstream oss;
            oss << std::this_thread::get_id() << std::endl;
            REprintf("Thread ID: %s\n", oss.str().c_str());
        }

        LRESULT CALLBACK callbackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
            case WM_TIMER:
                print_thread_id();
                REprintf("Timer callback!\n");
                return 0;
            case HTTPGD_MESSAGE_ID:
                print_thread_id();
                REprintf("httpgd calls!\n");
                return 0;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }

        class ServiceTimer {
        public:
            ServiceTimer() {
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

                REprintf("Initialized!\n");


                SetTimer(m_hwind,      // handle to main window
                            IDT_TIMER1, // timer identifier
                            10000,      // 5-second interval
                            NULL);      // no timer callback
            }

            ~ServiceTimer() {
                REprintf("Destroyed!\n");
                KillTimer(m_hwind, IDT_TIMER1);
                DestroyWindow(m_hwind);
                REprintf("done!\n");
            }

            void message() {
                //Use RegisterWindowMessage for guaranteed unique message id and PostMessage for non-blocking
                SendMessage(m_hwind, HTTPGD_MESSAGE_ID, NULL, NULL);
            }

        private:
            

            static const UINT_PTR IDT_TIMER1 = 1;
            HWND m_hwind;

        };

        static ServiceTimer service_timer;



        void init()
        {
            print_thread_id();

            service_timer.message();
            
            std::thread t1([](){ 
                print_thread_id();
                std::this_thread::sleep_for(std::chrono::seconds(3));
                service_timer.message();
            });

            t1.detach();

            //initialize();

            //auto a = submit([](){ Rprintf("hello!\n"); return true; }).get();

        }

    } // namespace service
} // namespace httpgd

#endif