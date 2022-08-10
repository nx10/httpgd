#include <cpp11/R.hpp>
#include <cpp11/protect.hpp>

[[cpp11::init]]
void import_unigd_api(DllInfo* dll) 
{
    /*if (!unigd::load_api())
    {
        Rf_error("ERROR: 'httpgd' was compiled with a version of 'unigd' that is incompatible with the one currently installed. Please install or compile matching versions.");
        cpp11::stop("");
    }*/
}
