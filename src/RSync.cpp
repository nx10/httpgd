

#include <mutex>
#include <later_api.h>
#include "RSync.h"

namespace httpgd
{
    namespace rsync
    {
        std::mutex later_mutex;

        struct RSyncData
        {
            void (*func)(void *);
            void *data;
        } rsdat;

        void later(void (*func)(void *), void *data, double secs)
        {
            later_mutex.lock();
            rsdat.data = data;
            rsdat.func = func;
            later::later([](void *data) {
                try {
                    auto d = static_cast<RSyncData *>(data);
                    d->func(d->data);
                } catch (const std::exception& e) { } // make sure mutex gets unlocked; todo: log exceptions
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
