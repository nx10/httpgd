#ifndef HTTPGD_RSYNC_H
#define HTTPGD_RSYNC_H

namespace httpgd
{
    namespace asynclater2
    {
        // Thread safe later
        void later(void (*func)(void *), void *data, double secs);
        void awaitLater();
    } // namespace asynclater
} // namespace httpgd

#endif