#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace httpgd
{
    template<class T>
    inline boost::optional<T> param_to(const char *t_param)
    {
        if (t_param == nullptr)
        {
            return boost::none;
        }
        try
        {
            return boost::lexical_cast<T>(t_param);
        }
        catch (...)
        {
        }
        return boost::none;
    }
    template<>
    inline boost::optional<std::string> param_to(const char *t_param)
    {
        if (t_param == nullptr)
        {
            return boost::none;
        }
        return std::string(t_param);
    }
    template<>
    inline boost::optional<const char*> param_to(const char *t_param)
    {
        if (t_param == nullptr)
        {
            return boost::none;
        }
        return t_param;
    }
} // namespace httpgd
