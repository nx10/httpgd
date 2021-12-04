#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <string>
#include <vector>

namespace httpgd
{
    namespace compr
    {
        std::vector<unsigned char> compress_str(const std::string &s);

    } // namespace compress

} // namespace httpgd

#endif // COMPRESSION_H