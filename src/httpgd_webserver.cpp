
#include "httpgd_webserver.h"

#define CROW_MAIN
#include <crow.h>
#include <fmt/format.h>
#include <compat/optional.hpp>

#include <memory>

#include "httpgd_version.h"
#include "optional_lex.h"

namespace httpgd
{
namespace web
{
namespace
{
const char *HTTPGD_CLIENT_INFO = "httpgd " HTTPGD_VERSION;

inline std::experimental::optional<std::string> read_txt(const std::string &filepath)
{
  std::ifstream t(filepath);
  if (t.fail())
  {
    return std::experimental::nullopt;
  }
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

inline crow::json::wvalue device_state_json(const unigd_device_state &state)
{
  return crow::json::wvalue(
      {{"upid", state.upid}, {"hsize", state.hsize}, {"active", state.active}});
}

struct plot_return : public crow::returnable
{
  std::string dump() const override
  {
    return std::string(m_render.buffer, m_render.buffer + m_render.size);
  }

  plot_return(const unigd_renderer_info &t_info, unigd_render_access t_render)
      : crow::returnable(t_info.mime), m_render(std::move(t_render))
  {
  }

  ~plot_return() = default;

 private:
  const unigd_render_access m_render;
};

inline std::experimental::optional<UNIGD_PLOT_ID> req_find_id(unigd_api_v1 *api,
                                                  UNIGD_HANDLE ugd_handle,
                                                  const crow::request &req)
{
  if (!api)
  {
    return std::experimental::nullopt;
  }
  const auto p_id = param_to<UNIGD_PLOT_ID>(req.url_params.get("id"));
  if (p_id)
  {
    return p_id;
  }
  const auto p_index = param_to<UNIGD_PLOT_ID>(req.url_params.get("index"));
  if (!p_index)
  {
    return std::experimental::nullopt;
  }

  std::experimental::optional<UNIGD_PLOT_ID> re = std::experimental::nullopt;
  unigd_find_results qr;
  const auto handle = api->device_plots_find(ugd_handle, *p_index, 1, &qr);
  if (qr.size > 0)
  {
    re = qr.ids[0];
  }
  api->device_plots_find_destroy(handle);
  return re;
}

}  // namespace

void HttpgdLogHandler::log(std::string message, crow::LogLevel level)
{
  std::string prefix;
  switch (level)
  {
    case crow::LogLevel::Debug:
      prefix = "DEBUG   ";
      break;
    case crow::LogLevel::Info:
      prefix = "INFO    ";
      break;
    case crow::LogLevel::Warning:
      prefix = "WARNING ";
      break;
    case crow::LogLevel::Error:
      prefix = "ERROR   ";
      break;
    case crow::LogLevel::Critical:
      prefix = "CRITICAL";
      break;
  }
  // if (m_api)
  //     m_api->log(std::string("(") + timestamp() + std::string(") [") + prefix +
  //     std::string("] ") + message);
}

std::string HttpgdLogHandler::timestamp()
{
  char date[32];
  time_t t = time(0);

  tm my_tm;

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifdef CROW_USE_LOCALTIMEZONE
  localtime_s(&my_tm, &t);
#else
  gmtime_s(&my_tm, &t);
#endif
#else
#ifdef CROW_USE_LOCALTIMEZONE
  localtime_r(&t, &my_tm);
#else
  gmtime_r(&t, &my_tm);
#endif
#endif

  size_t sz = strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &my_tm);
  return std::string(date, date + sz);
}

void WebServer::TokenGuard::before_handle(crow::request &req, crow::response &res,
                                          context &ctx)
{
  if (!m_use_token)
  {
    return;
  }
  std::experimental::optional<std::string> user_token = std::experimental::nullopt;
  const auto f_header_token = req.headers.find("X-HTTPGD-TOKEN");
  if (f_header_token != req.headers.end())
  {
    user_token = f_header_token->second;
  }
  else
  {
    user_token = param_to<std::string>(req.url_params.get("token"));
  }

  if (!user_token || (user_token.value() != m_token))
  {
    res.code = crow::UNAUTHORIZED;
    res.end();
  }
}
void WebServer::TokenGuard::after_handle(crow::request &req, crow::response &res,
                                         context &ctx)
{
}

WebServer::WebServer(const HttpgdServerConfig &t_conf)
    : m_conf(t_conf), m_app(), m_mtx_update_subs(), m_update_subs()
{
  m_client.close = [](void *client_data)
  { static_cast<WebServer *>(client_data)->device_close(); };
  m_client.start = [](void *client_data)
  { static_cast<WebServer *>(client_data)->device_start(); };
  m_client.state_change = [](void *client_data)
  { static_cast<WebServer *>(client_data)->device_state_change(); };
  m_client.info = [](void *client_data) { return HTTPGD_CLIENT_INFO; };
}

bool WebServer::attach(int devnum)
{
  if (m_api == nullptr)
  {
    m_api = ugd::api;
  }

  m_ugd_handle = m_api->device_attach(devnum, &m_client, ugd::httpgd_client_id, this);

  return m_ugd_handle != nullptr;
}

std::string WebServer::status_info()
{
  const auto ws_count = m_update_subs.size();
  return fmt::format("unigd: {}; httpgd: " HTTPGD_VERSION "; WebSocket connections: {}",
                     m_api->info(), ws_count);
}

const HttpgdServerConfig &WebServer::get_config() { return m_conf; }

unsigned short WebServer::port()
{
  m_app.wait_for_server_start();
  return m_app.port();
}

void WebServer::device_start() { m_server_thread = std::thread(&WebServer::run, this); }

void WebServer::run()
{
  crow::logger::setHandler(&m_log_handler);

  if (m_conf.cors)
  {
    auto &cors = m_app.get_middleware<crow::CORSHandler>();
    cors.global().headers("Access-Control-Allow-Origin", "*");
  }

  if (m_conf.use_token)
  {
    auto &middle = m_app.get_middleware<TokenGuard>();
    middle.m_use_token = true;
    middle.m_token = m_conf.token;
  }

  // CROW_ROUTE(m_app, "/")
  // ([]()
  //  { return "httpgd server is running!"; });

  CROW_ROUTE(m_app, "/live")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&](const crow::request &, crow::response &res)
          {
            const auto filepath = std::string(m_conf.wwwpath) + "/index.html";
            res.set_static_file_info_unsafe(filepath);
            res.end();
          });

  CROW_ROUTE(m_app, "/state")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&]()
          {
            if (m_api)
            {
              const auto state = m_api->device_state(m_ugd_handle);
              return crow::response(device_state_json(state));
            }
            return crow::response(crow::status::NOT_FOUND);
          });

  CROW_ROUTE(m_app, "/renderers")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&]()
          {
            unigd_renderers_list renderers;
            if (m_api)
            {
              auto renderers_handle = m_api->renderers(&renderers);

              std::vector<crow::json::wvalue> a;
              a.reserve(renderers.size);
              for (uint64_t i = 0; i < renderers.size; ++i)
              {
                const auto &ren = renderers.entries[i];
                a.push_back(crow::json::wvalue({{"id", ren.id},
                                                {"mime", ren.mime},
                                                {"ext", ren.fileext},
                                                {"name", ren.name},
                                                {"type", ren.type},
                                                {"bin", !ren.text},
                                                {"descr", ren.description}}));
              }

              m_api->renderers_destroy(renderers_handle);
              return crow::response(crow::json::wvalue({{"renderers", a}}));
            }
            return crow::response(crow::status::NOT_FOUND);
          });

  CROW_ROUTE(m_app, "/plots")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&](const crow::request &req)
          {
            const auto p_index = param_to<int>(req.url_params.get("index"));
            const auto p_limit = param_to<int>(req.url_params.get("limit"));

            if (m_api)
            {
              UNIGD_FIND_HANDLE find_handle;
              unigd_find_results qr;
              find_handle = m_api->device_plots_find(
                  m_ugd_handle, p_index.value_or(0), p_limit.value_or(0), &qr);

              std::vector<crow::json::wvalue> plot_list;
              plot_list.reserve(qr.size);
              for (UNIGD_PLOT_INDEX i = 0; i < qr.size; ++i)
              {
                plot_list.push_back(
                    crow::json::wvalue({{"id", fmt::format("{}", qr.ids[i])}}));
              }
              const auto sj = device_state_json(qr.state);
              m_api->device_plots_find_destroy(find_handle);

              return crow::response(
                  crow::json::wvalue({{"state", sj}, {"plots", plot_list}}));
            }
            return crow::response(crow::status::NOT_FOUND);
          });

  CROW_ROUTE(m_app, "/plot")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&](const crow::request &req)
          {
            const auto p_width = param_to<int>(req.url_params.get("width"));
            const auto p_height = param_to<int>(req.url_params.get("height"));
            double width, height, zoom;
            if (p_width && p_height)
            {
              zoom = param_to<double>(req.url_params.get("zoom")).value_or(1);
              width = (*p_width) / zoom;
              height = (*p_height) / zoom;
            }
            else
            {
              zoom = 1;
              width = p_width.value_or(-1);
              height = p_height.value_or(-1);
            }
            const auto p_id = req_find_id(m_api, m_ugd_handle, req).value_or(-1);
            const auto p_renderer =
                param_to<std::string>(req.url_params.get("renderer")).value_or("svg");
            const auto p_download =
                param_to<const char *>(req.url_params.get("download"));
            if (m_api)
            {
              unigd_renderer_info rinfo;
              auto rinfo_handle = m_api->renderers_find(p_renderer.c_str(), &rinfo);

              if (!rinfo_handle)
              {
                return crow::response(crow::status::NOT_FOUND);
              }

              unigd_render_access render;
              auto render_handle = m_api->device_render_create(
                  m_ugd_handle, p_renderer.c_str(), p_id, {width, height, zoom}, &render);

              if (!render_handle)
              {
                m_api->device_render_destroy(render_handle);
                m_api->renderers_find_destroy(rinfo_handle);
                return crow::response(crow::status::NOT_FOUND);
              }

              auto res = crow::response(plot_return(rinfo, std::move(render)));
              m_api->device_render_destroy(render_handle);
              m_api->renderers_find_destroy(rinfo_handle);

              if (p_download)
              {
                res.add_header("Content-Disposition",
                               fmt::format("attachment; filename=\"{}\"", *p_download));
              }
              return res;
            }

            return crow::response(crow::status::NOT_FOUND);
          });

  CROW_ROUTE(m_app, "/info")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&]()
          {
            return crow::json::wvalue({{"id", m_conf.id},
                                       {"version", "httpgd " HTTPGD_VERSION},
                                       {"unigd", m_api ? m_api->info() : ""}});
          });

  CROW_ROUTE(m_app, "/remove")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&](const crow::request &req)
          {
            const auto p_id = req_find_id(m_api, m_ugd_handle, req);
            if (!p_id)
            {
              return crow::response(crow::status::NOT_FOUND);
            }
            if (m_api)
            {
              if (m_api->device_plots_remove(m_ugd_handle, *p_id))
              {
                const auto state = m_api->device_state(m_ugd_handle);
                return crow::response(device_state_json(state));
              }
            }
            return crow::response(crow::status::NOT_FOUND);
          });

  CROW_ROUTE(m_app, "/clear")
      .CROW_MIDDLEWARES(m_app, TokenGuard)(
          [&]()
          {
            if (!m_api)
            {
              return crow::response(crow::status::INTERNAL_SERVER_ERROR);
            }
            if (m_api->device_plots_clear(m_ugd_handle))
            {
              const auto state = m_api->device_state(m_ugd_handle);
              return crow::response(device_state_json(state));
            }
            return crow::response(crow::status::NOT_FOUND);
          });

  CROW_WEBSOCKET_ROUTE(m_app, "/")
      .onopen(
          [&](crow::websocket::connection &conn)
          {
            CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
            std::lock_guard<std::mutex> _(m_mtx_update_subs);
            m_update_subs.insert(&conn);
          })
      .onclose(
          [&](crow::websocket::connection &conn, const std::string &reason)
          {
            CROW_LOG_INFO << "websocket connection closed: " << reason;
            std::lock_guard<std::mutex> _(m_mtx_update_subs);
            m_update_subs.erase(&conn);
          })
      .onmessage(
          [&](crow::websocket::connection & /*conn*/, const std::string &data,
              bool is_binary)
          {
            std::lock_guard<std::mutex> _(m_mtx_update_subs);
            for (auto u : m_update_subs)
              if (is_binary)
                u->send_binary(data);
              else
                u->send_text(data);
          });

  CROW_ROUTE(m_app,
             "/<str>")  // No token guard so static resources can be included in html
  (
      [&](crow::response &res, std::string s)
      {
        CROW_LOG_INFO << "static: " << s;
        const auto filepath = std::string(m_conf.wwwpath) + "/" + s;
        res.set_static_file_info_unsafe(filepath);
        res.end();
      });

  m_app.bindaddr(m_conf.host).port(m_conf.port).multithreaded().run();
}

void WebServer::device_close()
{
  
  //for (auto u : m_update_subs)
  //{
  //  u->userdata();
  //}

  m_app.stop();

  if (m_server_thread.joinable())
  {
    m_server_thread.join();
  }

  if (m_api && m_ugd_handle)
  {
    m_api->device_destroy(m_ugd_handle);
  }

  delete this;  // attention!
}

void WebServer::broadcast_state(const unigd_device_state &t_state)
{
  std::lock_guard<std::mutex> _(m_mtx_update_subs);
  for (auto u : m_update_subs)
  {
    u->send_text(device_state_json(t_state).dump());
  }
}

void WebServer::device_state_change()
{
  if (!m_api)
  {
    return;
  }
  const auto state = m_api->device_state(m_ugd_handle);
  broadcast_state(state);
}

}  // namespace web
}  // namespace httpgd
