
#ifndef HTTPGD_RSYNC_BACKGROUNDTASK_H
#define HTTPGD_RSYNC_BACKGROUNDTASK_H

#include <thread>

namespace httpgd
{
    namespace rsync
    {
        // This is a direct replacement for later::BackgroundTask.
        // The later implementation keeps crashing when its destructor 
        // is called and I dont know why.
        // Weirdly this implementation crashes when it is used outside
        // of the graphics device code.
        class BackgroundTask
        {
        public:
            virtual ~BackgroundTask()
            {
                if (m_background_thread.joinable())
                {
                    m_background_thread.join();
                }
            }

            void begin()
            {
                m_background_thread = std::thread(&BackgroundTask::m_run_task, this);
            }

        protected:
            virtual void execute(){};
            virtual void complete(){};

        private:
            std::thread m_background_thread;
            void m_run_task()
            {
                execute();
                complete();
            }
        };
    } // namespace rsync
} // namespace httpgd

#endif