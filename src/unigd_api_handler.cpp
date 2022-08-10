
#include "unigd_api_handler.h"

namespace httpgd
{
    unigd_api_access::unigd_api_access() {
        unigd_api_v1_create(&api);
    }

    unigd_api_access::~unigd_api_access() {
        unigd_api_v1_destroy(api);
    }

} // namespace httpgd

