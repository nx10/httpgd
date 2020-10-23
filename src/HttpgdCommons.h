
#ifndef HTTPGD_COMMONS_H
#define HTTPGD_COMMONS_H

#include <string>

namespace httpgd
{
    struct HttpgdState {
        int upid;
        size_t hsize;
        bool active;
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
    };

}

#endif