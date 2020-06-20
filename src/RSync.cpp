

#include <mutex>
#include <later_api.h>
#include "RSync.h"

namespace httpgd
{
    namespace rsync
    {
        std::mutex sync_mutex;
        std::mutex later_mutex;

        struct RSyncData
        {
            void (*func)(void *);
            void *data;
        } rsdat;

        void lock()
        {
            sync_mutex.lock();
        }
        void unlock()
        {
            sync_mutex.unlock();
        }

        void later(void (*func)(void *), void *data, double secs)
        {
            later_mutex.lock();
            rsdat.data = data;
            rsdat.func = func;
            later::later([](void *data) {
                auto d = static_cast<RSyncData *>(data);
                d->func(d->data);
                later_mutex.unlock();
            },
                         &rsdat, secs);
        }

        void awaitLater()
        {
            later_mutex.lock();
            later_mutex.unlock();
        }
    } // namespace rsync
} // namespace httpgd
