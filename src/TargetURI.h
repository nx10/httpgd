#ifndef HTTPGD_TARGET_URI_H
#define HTTPGD_TARGET_URI_H

#include <map>

namespace httpgd
{
    namespace web
    {
        // This should be replaced with the URI class in boost/networking
        // https://cpp-netlib.org/0.10.1/in_depth/uri.html
        // when it is available in BH, or with the beast URI parser
        // https://github.com/boostorg/beast/issues/787
        // when it gets released.
        class TargetURI
        {
        public:
            std::string path;
            std::map<std::string, std::string> qparams;

            TargetURI(const std::string &uri);

        private:
            // cuts off everything after ? or #
            std::string findpath(const std::string &uri);
            std::map<std::string, std::string> qparse(const std::string &query);
        };
    } // namespace http
} // namespace httpgd

#endif