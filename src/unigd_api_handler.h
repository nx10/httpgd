#ifndef HTTPGD_UNIGD_API_HANDLER_H
#define HTTPGD_UNIGD_API_HANDLER_H

#include <unigd_api.h>

namespace httpgd
{
    struct unigd_api_access {
        unigd_api_access();
        ~unigd_api_access();
        unigd_api_v1 *api;
    };

} // namespace httpgd


#endif

