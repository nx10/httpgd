#ifndef HTTPGD_HTTP_CONNECTION_H
#define HTTPGD_HTTP_CONNECTION_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "HttpgdHttpTask.h"
#include "HttpgdDataStore.h"

namespace httpgd
{
    namespace http
    {

        namespace beast = boost::beast;   // from <boost/beast.hpp>
        namespace net = boost::asio;      // from <boost/asio.hpp>
        using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

        // Accepts incoming connections and launches the sessions
        class listener : public std::enable_shared_from_this<listener>
        {
            net::io_context &ioc_;
            tcp::acceptor acceptor_;
            std::shared_ptr<HttpgdServerConfig> conf;
            std::shared_ptr<HttpgdDataStore> dstore;

        public:
            listener(
                net::io_context &ioc,
                tcp::endpoint endpoint,
                std::shared_ptr<HttpgdServerConfig> t_conf,
                std::shared_ptr<HttpgdDataStore> t_dstore);

            unsigned short get_port_after_bind();

            // Start accepting incoming connections
            void
            run();


        private:
            void
            do_accept();

            void
            on_accept(beast::error_code ec, tcp::socket socket);
        };

    } // namespace http
} // namespace httpgd

#endif