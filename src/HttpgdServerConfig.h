
#ifndef HTTPGD_SERVER_CONFIG_H
#define HTTPGD_SERVER_CONFIG_H

#include <string>

namespace httpgd
{
    struct HttpgdServerConfig
    {
        std::string host;
        int port;
        std::string livehtml;
        bool cors;
        bool use_token;
        std::string token;
        bool record_history;
    };

}

#endif