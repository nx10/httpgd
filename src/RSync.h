#ifndef HTTPGD_RSYNC_H
#define HTTPGD_RSYNC_H

namespace httpgd
{
    namespace rsync
    {
        //void lock();
        //void unlock();
        void later(void (*func)(void *), void *data, double secs);
        void awaitLater();
    } // namespace rsync
} // namespace httpgd

#endif