#ifndef HTTPGD_HTTPGD_API_H
#define HTTPGD_HTTPGD_API_H

#include <string>
#include <memory>
#include "HttpgdCommons.h"

namespace httpgd
{
    class HttpgdApi
    {
    public:
        virtual void api_render(int index, double width, double height) = 0;
        virtual bool api_remove(int index) = 0;
        virtual bool api_clear() = 0;

        virtual void api_svg(std::ostream &os, int index, double width, double height) = 0;

        //virtual int api_upid() = 0;
        //virtual bool api_active() = 0;
        //virtual int api_page_count() = 0;
        virtual HttpgdState api_state() = 0;
        
        virtual std::shared_ptr<HttpgdServerConfig> api_server_config() = 0;
    };
} // namespace httpgd

#endif