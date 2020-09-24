// [[Rcpp::plugins("cpp11")]]

#include <Rcpp.h>

//#include <R_ext/GraphicsEngine.h>

#include <vector>
#include <string>
#include <fstream>

#include "lib/svglite_utils.h"

#include "HttpgdDev.h"
#include "DrawData.h"

#include "RSync.h"

#define LOGDRAW 0

namespace httpgd
{

    // returns system path to {package}/inst/www/{filename}
    std::string get_wwwpath(const std::string &filename)
    {
        Rcpp::Environment base("package:base");
        Rcpp::Function sys_file = base["system.file"];
        Rcpp::StringVector res = sys_file("www", filename,
                                          Rcpp::_["package"] = "httpgd");
        return std::string(res[0]);
    }

    inline HttpgdDev *getDev(pDevDesc dd)
    {
        return static_cast<HttpgdDev *>(dd->deviceSpecific);
    }

    // --------------------------------------

} // namespace httpgd

// [[Rcpp::export]]
bool httpgd_(std::string host, int port, std::string bg, double width, double height,
             double pointsize, Rcpp::List aliases, bool cors, std::string token, bool webserver, bool silent)
{
    bool recording = true;
    bool use_token = token.length();
    int ibg = R_GE_str2col(bg.c_str());

    std::string wwwpath(httpgd::get_wwwpath(""));

    auto dev = new httpgd::HttpgdDev(
        {host,
         port,
         wwwpath,
         cors,
         use_token,
         token,
         recording,
         webserver,
         silent},
        {ibg,
         width,
         height,
         pointsize,
         aliases});

    httpgd::HttpgdDev::make_device("httpgd", dev);
    return dev->server_start();
}

inline httpgd::HttpgdDev *validate_httpgddev(int devnum)
{
    if (devnum < 1 || devnum > 64) // R_MaxDevices
    {
        Rcpp::stop("invalid graphical device number");
    }

    pGEDevDesc gdd = GEgetDevice(devnum - 1);
    if (!gdd)
    {
        Rcpp::stop("invalid device");
    }
    pDevDesc dd = gdd->dev;
    if (!dd)
    {
        Rcpp::stop("invalid device");
    }
    auto dev = static_cast<httpgd::HttpgdDev *>(dd->deviceSpecific);
    if (!dev)
    {
        Rcpp::stop("invalid device");
    }

    return dev;
}

// [[Rcpp::export]]
Rcpp::List httpgd_state_(int devnum)
{
    auto dev = validate_httpgddev(devnum);

    auto svr_config = dev->api_server_config();

    return Rcpp::List::create(
        Rcpp::Named("host") = svr_config->host,
        Rcpp::Named("port") = dev->server_port(),
        Rcpp::Named("token") = svr_config->token,
        Rcpp::Named("hsize") = dev->api_page_count(),
        Rcpp::Named("upid") = dev->api_upid(),
        Rcpp::Named("active") = dev->device_active());
}

// [[Rcpp::export]]
std::string httpgd_random_token_(int len)
{
    if (len < 0)
    {
        Rcpp::stop("Length needs to be 0 or higher.");
    }
    return httpgd::HttpgdDev::random_token(len);
}

// [[Rcpp::export]]
std::string httpgd_svg_(int devnum, int page, double width, double height)
{
    auto dev = validate_httpgddev(devnum);

    std::ostringstream buf;
    dev->api_svg(buf, page, width, height);
    return buf.str();
}

// [[Rcpp::export]]
bool httpgd_remove_(int devnum, int page)
{
    auto dev = validate_httpgddev(devnum);
    return dev->api_remove(page);
}

// [[Rcpp::export]]
bool httpgd_clear_(int devnum)
{
    auto dev = validate_httpgddev(devnum);
    return dev->api_clear();
}