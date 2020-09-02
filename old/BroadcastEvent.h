#ifndef advfujcbiko
#define advfujcbiko

#include <string>

namespace httpgd
{
    class Broadcastable
    {
    public:
        virtual void broadcast(std::string message) {};
    };
} // namespace httpgd

#endif