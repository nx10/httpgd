#include "HttpgdCompress.h"

#include <string>
#include <vector>
#include <zlib.h>

namespace httpgd
{
    namespace compr
    {
        template<typename charTypeIn, typename charTypeOut>
        static std::vector<charTypeOut> compressToGzip(const charTypeIn *input, size_t inputSize)
        {
            static_assert(sizeof(charTypeIn) == 1, "input not a char type");
            static_assert(sizeof(charTypeOut) == 1, "output not a char type vector");

            z_stream zs;
            zs.zalloc = Z_NULL;
            zs.zfree = Z_NULL;
            zs.opaque = Z_NULL;
            zs.avail_in = static_cast<uInt>(inputSize);
            zs.next_in = (Bytef *)input;

            int ret = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
            if (ret != Z_OK) {
                return std::vector<charTypeOut> {};
            }
            
            std::vector<charTypeOut> buffer;

            for (;;) {
                const int chunk_size = 16384;
                const size_t old_size = buffer.size();
                buffer.resize(buffer.size() + chunk_size);

                zs.avail_out = chunk_size;
                zs.next_out = reinterpret_cast<Bytef *>(&buffer[old_size]);
                ret = deflate(&zs, Z_FINISH);
                if (ret == Z_STREAM_ERROR) {
                    deflateEnd(&zs);
                    return std::vector<charTypeOut> {};
                } 
                buffer.resize(old_size + (chunk_size - zs.avail_out));
                
                if (zs.avail_out != 0) {
                    break;
                }
            }
            deflateEnd(&zs);
            return buffer;
        }

        std::vector<unsigned char> compress_str(const std::string &s)
        {
            return compressToGzip<char, unsigned char>(s.c_str(), s.size());
        }

    } // namespace compress

} // namespace httpgd
