
#include <thread>

namespace httpgd::rsync
{
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
        virtual void execute() {};
        virtual void complete() {};

    private:
        std::thread m_background_thread;
        void m_run_task()
        {
            execute();
            complete();
        }
    };
} // namespace httpgd::rsync