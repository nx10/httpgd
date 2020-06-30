
#include <regex>

#include "TargetURI.h"

namespace httpgd
{
    namespace http
    {
        TargetURI::TargetURI(const std::string &uri)
        {
            path = findpath(uri);
            qparams = qparse(uri);
        }
        
        std::string TargetURI::findpath(const std::string &uri)
        {
            size_t pos_q = uri.find_first_of('?');
            size_t pos_h = uri.find_first_of('#');
            if (pos_q == std::string::npos)
            {
                pos_q = uri.length();
            }
            if (pos_h == std::string::npos)
            {
                pos_h = uri.length();
            }
            return uri.substr(0, std::min<size_t>(pos_q, pos_h));
        }

        std::map<std::string, std::string> TargetURI::qparse(const std::string &uri)
        {
            std::map<std::string, std::string> data;
            std::regex pattern("([\\w+%]+)=([^&]*)");
            auto words_begin = std::sregex_iterator(uri.begin(), uri.end(), pattern);
            auto words_end = std::sregex_iterator();

            for (std::sregex_iterator i = words_begin; i != words_end; i++)
            {
                std::string key = (*i)[1].str();
                std::string value = (*i)[2].str();
                data[key] = value;
            }

            return data;
        }

    } // namespace http
} // namespace httpgd
