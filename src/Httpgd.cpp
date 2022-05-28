
#include <cpp11/function.hpp>
#include <cpp11/strings.hpp>
#include <cpp11/list.hpp>
#include <cpp11/integers.hpp>
#include <cpp11/logicals.hpp>
#include <cpp11/data_frame.hpp>
#include <cpp11/as.hpp>
#include <cpp11/raws.hpp>

//#include <R_ext/GraphicsEngine.h>

#include <vector>
#include <string>

#include <unigd_api.h>

#include "HttpgdRng.h"
#include "HttpgdVersion.h"

#include <boost/optional.hpp>

#include "HttpgdWebServer.h"

namespace httpgd
{

    // returns system path to {package}/inst/www/{filename}
    std::string get_wwwpath(const std::string &filename)
    {
        using namespace cpp11::literals;

        auto sys_file = cpp11::package("base")["system.file"];

        cpp11::strings res(sys_file("www", filename,
                                          "package"_nm = "httpgd"));
        return res[0];
    }

    // --------------------------------------

} // namespace httpgd

[[cpp11::register]]
bool httpgd_(int devnum, std::string host, int port, bool cors, std::string token, 
             bool webserver, bool silent, bool fix_text_width, std::string extra_css)
{
    bool recording = true;
    bool use_token = token.length();

    std::string wwwpath(httpgd::get_wwwpath(""));

    boost::optional<std::string> css;
    if (!extra_css.empty()) 
    {
        css = extra_css;
    }

    const httpgd::web::HttpgdServerConfig conf{host,
         port,
         wwwpath,
         cors,
         use_token,
         token,
         recording,
         webserver,
         silent,
         httpgd::rng::uuid()};

    return ugd_attach_client(devnum, std::make_shared<httpgd::web::WebServer>(conf));
}

[[cpp11::register]]
std::string httpgd_random_token_(int len)
{
    if (len < 0)
    {
        cpp11::stop("Length needs to be 0 or higher.");
    }
    return httpgd::rng::token(len);
}