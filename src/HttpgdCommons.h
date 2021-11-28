
#ifndef HTTPGD_COMMONS_H
#define HTTPGD_COMMONS_H

#include <limits>
#include <string>
#include <vector>

namespace httpgd
{
    // safely increases numbers (wraps to 0)
    template <typename T>
    T incwrap(T t_value)
    {
        T v = t_value;
        if (v == std::numeric_limits<T>::max())
        {
            return static_cast<T>(0);
        }
        return v + 1;
    }

    struct HttpgdState {
        int upid;
        size_t hsize;
        bool active;
    };

    struct HttpgdQueryResults {
        HttpgdState state;
        std::vector<int32_t> ids;
    };

    struct HttpgdServerConfig
    {
        std::string host;
        int port;
        std::string wwwpath;
        bool cors;
        bool use_token;
        std::string token;
        bool record_history;
        bool webserver;
        bool silent;
        std::string id;
    };

} // namespace httpgd

#endif