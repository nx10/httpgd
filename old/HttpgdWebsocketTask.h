
#include "HttpgdWebsocket.h"
#include "BackgroundTask.h"

#include "Broadcast.h"

namespace httpgd
{
    namespace ws
    {
        class HttpgdWebsocketTask : public rsync::BackgroundTask, public Broadcastable
        {
        public:
            HttpgdWebsocketTask(const std::string &t_host, unsigned short t_port, int t_threads);
            HttpgdWebsocketTask(int t_taskid, net::io_context *t_pioc);
            void end();
            int await_port();
            void broadcast(std::string message) override;

        protected:
            void execute();
            void complete();

        private:
            int taskid;
            std::string host;
            unsigned short port;
            int threads;
            net::io_context *pioc;
            
            std::atomic<bool> m_port_bound; // server is bound to a port
            unsigned int m_port;
            std::shared_ptr<Broadcastable> m_lis;
        };
    } // namespace ws
} // namespace rbeast

/*
void beast_websocket_server_async(std::string host = "127.0.0.1", int port = 1234, int threads = 4)
{
    if (rbeast::ws::ws_server == nullptr)
    {
        rbeast::ws::ws_server = new rbeast::ws::WebsocketServerAsyncTask(host, port, threads);
        rbeast::ws::ws_server->begin();

        Rcpp::Rcout << "WS Server started: ws://" << host.c_str() << ":" << port << "/\n";
    }
    else
    {
        Rcpp::Rcout << "WS Server already running, call beast_http_async_stop()";
    }
}

void beast_websocket_server_async_stop()
{
    if (rbeast::ws::ws_server != nullptr)
    {
        rbeast::ws::ws_server->end();
    }
}

*/