#ifndef __UNIGD_HTTPGD_WEBSERVER_H__
#define __UNIGD_HTTPGD_WEBSERVER_H__

#include <crow.h>
#include <crow/middlewares/cors.h>

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "unigd_impl.h"

namespace httpgd
{
namespace web
{
struct HttpgdServerConfig
{
  std::string host;
  int port;
  std::string wwwpath;
  bool cors;
  bool use_token;
  std::string token;
  bool record_history;
  bool silent;
  std::string id;
};

class HttpgdLogHandler : public crow::ILogHandler
{
 public:
  void log(std::string message, crow::LogLevel level) override;

 private:
  static std::string timestamp();
};

class WebServer
{
  struct TokenGuard : crow::ILocalMiddleware
  {
    struct context
    {
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx);

    void after_handle(crow::request& req, crow::response& res, context& ctx);

    bool m_use_token = false;
    std::string m_token;
  };

 public:
  WebServer(const HttpgdServerConfig& t_config);

  bool attach(int devnum);

  void device_start();
  void device_close();
  void device_state_change();

  std::string status_info();
  const HttpgdServerConfig& get_config();

  unsigned short port();
  void broadcast_state(const unigd_device_state& state);

 private:
  unigd_api_v1* m_api = nullptr;
  UNIGD_HANDLE m_ugd_handle = nullptr;
  unigd_graphics_client m_client;

  HttpgdServerConfig m_conf;
  crow::App<crow::CORSHandler, TokenGuard> m_app;
  HttpgdLogHandler m_log_handler;
  std::mutex m_mtx_update_subs;
  std::unordered_set<crow::websocket::connection*> m_update_subs;
  int m_last_upid = -1;
  bool m_last_active = true;
  std::thread m_server_thread;

  void run();
};
}  // namespace web
}  // namespace httpgd

#endif /* __UNIGD_HTTPGD_WEBSERVER_H__ */
