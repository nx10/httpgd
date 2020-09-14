#ifndef HTTPGD_SERVER_STORE_H
#define HTTPGD_SERVER_STORE_H

#include <functional>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>

#include "HttpgdServerConfig.h"
#include "HttpgdApi.h"
#include "DrawData.h"

namespace httpgd
{
    struct HttpgdDataStorePageSize {
        double width;
        double height;
    };

    class HttpgdDataStore
    {
    public:
        
        HttpgdDataStore();
        ~HttpgdDataStore();
        
        bool diff(int index, double width, double height);
        void svg(std::string *buf, int index);

        int count();

        int append(double width, double height);
        void clear(int index, bool silent);
        bool remove(int index, bool silent);
        bool remove_all();
        void resize(int index, double width, double height);
        HttpgdDataStorePageSize size(int index);

        void fill(int index, int fill);
        void add_dc(int index, std::shared_ptr<dc::DrawCall> dc, bool silent);
        void clip(int index, double x0, double x1, double y0, double y1);

        int upid();

    private:
        std::mutex m_store_mutex;

        std::vector<dc::Page> m_pages;
        int m_upid;

        void m_inc_upid();

        inline bool m_valid_index(int index);
        inline size_t m_index_to_pos(int index);
        
    };

} // namespace httpgd

#endif // HTTPGD_SERVER_H