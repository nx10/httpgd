#include "unigd_impl.h"

#include <cpp11/R.hpp>
#include <cpp11/protect.hpp>

#include "unigd_api.h"

namespace httpgd
{
    namespace ugd
    {
        unigd_api_v1 *api = nullptr;
        UNIGD_CLIENT_ID httpgd_client_id = 0;

        namespace
        {
            class unigd_api_guard
            {
            public:
                ~unigd_api_guard()
                {
                    destroy();
                }

                void create()
                {
                    if (api == nullptr)
                        unigd_api_v1_create(&api);
                }
                void destroy()
                {
                    if (api != nullptr)
                    {
                        unigd_api_v1_destroy(api);
                        api = nullptr;
                    }
                }
            };

            unigd_api_guard api_guard;
        } // namespace
    } // namespace ugd
} // namespace httpgd

[[cpp11::init]] void import_unigd_api(DllInfo *dll)
{
    httpgd::ugd::api_guard.create();
    httpgd::ugd::httpgd_client_id = httpgd::ugd::api->register_client_id();
    /*if (!unigd::load_api())
    {
        Rf_error("ERROR: 'httpgd' was compiled with a version of 'unigd' that is incompatible with the one currently installed. Please install or compile matching versions.");
        cpp11::stop("");
    }*/
}
