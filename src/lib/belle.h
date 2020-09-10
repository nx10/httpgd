/*
                                    88888888
                                  888888888888
                                 88888888888888
                                8888888888888888
                               888888888888888888
                              888888  8888  888888
                              88888    88    88888
                              888888  8888  888888
                              88888888888888888888
                              88888888888888888888
                             8888888888888888888888
                          8888888888888888888888888888
                        88888888888888888888888888888888
                              88888888888888888888
                            888888888888888888888888
                           888888  8888888888  888888
                           888     8888  8888     888
                                   888    888

                                   OCTOBANANA

Belle
0.5.1

An HTTP / Websocket library in C++17 using Boost.Beast and Boost.ASIO.
https://octobanana.com/software/belle

Licensed under the MIT License
Copyright (c) 2018 Brett Robinson <https://octobanana.com/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef OB_BELLE_HH
#define OB_BELLE_HH

#define OB_BELLE_VERSION_MAJOR 0
#define OB_BELLE_VERSION_MINOR 5
#define OB_BELLE_VERSION_PATCH 1

// Config Begin

// compile with -DOB_BELLE_CONFIG_<OPT> or
// comment out defines to alter the library

#define OB_BELLE_CONFIG_SSL_OFF
#define OB_BELLE_CONFIG_CLIENT_OFF

// ssl support
#ifndef OB_BELLE_CONFIG_SSL_OFF
#define OB_BELLE_CONFIG_SSL_ON
#endif // OB_BELLE_CONFIG_SSL_OFF

// client support
#ifndef OB_BELLE_CONFIG_CLIENT_OFF
#define OB_BELLE_CONFIG_CLIENT_ON
#endif // OB_BELLE_CONFIG_CLIENT_OFF

// server support
#ifndef OB_BELLE_CONFIG_SERVER_OFF
#define OB_BELLE_CONFIG_SERVER_ON
#endif // OB_BELLE_CONFIG_SERVER_OFF

// Config End

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#ifdef OB_BELLE_CONFIG_SSL_ON
#include <boost/beast/websocket/ssl.hpp>
#endif // OB_BELLE_CONFIG_SSL_ON

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/bind_executor.hpp>

#ifdef OB_BELLE_CONFIG_CLIENT_ON
#include <boost/asio/connect.hpp>
#endif // OB_BELLE_CONFIG_CLIENT_ON

#ifdef OB_BELLE_CONFIG_SSL_ON
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#endif // OB_BELLE_CONFIG_SSL_ON

#include <boost/config.hpp>

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <csignal>

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <functional>
#include <regex>
#include <memory>
#include <chrono>
#include <utility>
#include <initializer_list>
#include <optional>
#include <limits>
#include <type_traits>
#include <thread>

namespace OB::Belle
{

// aliases
namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;

#ifdef OB_BELLE_CONFIG_SSL_ON
namespace ssl = boost::asio::ssl;
#endif // OB_BELLE_CONFIG_SSL_ON

using tcp = boost::asio::ip::tcp;
using error_code = boost::system::error_code;
using Method = boost::beast::http::verb;
using Status = boost::beast::http::status;
using Header = boost::beast::http::field;
using Headers = boost::beast::http::fields;

// Ordered_Map: an insert ordered map
// enables fast random lookup and insert ordered iterators
// unordered map stores key value pairs
// queue holds insert ordered iterators to each key in the unordered map
template<typename K, typename V>
class Ordered_Map
{
public:

  // map iterators
  using m_iterator = typename std::unordered_map<K, V>::iterator;
  using m_const_iterator = typename std::unordered_map<K, V>::const_iterator;

  // index iterators
  using i_iterator = typename std::deque<m_iterator>::iterator;
  using i_const_iterator = typename std::deque<m_const_iterator>::const_iterator;


  Ordered_Map()
  {
  }

  Ordered_Map(std::initializer_list<std::pair<K, V>> const& lst)
  {
    for (auto const& [key, val] : lst)
    {
      _it.emplace_back(_map.insert({key, val}).first);
    }
  }

  ~Ordered_Map()
  {
  }

  Ordered_Map& operator()(K const& k, V const& v)
  {
    auto it = _map.insert_or_assign(k, v);

    if (it.second)
    {
      _it.emplace_back(it.first);
    }

    return *this;
  }

  i_iterator operator[](std::size_t index)
  {
    return _it[index];
  }

  i_const_iterator const operator[](std::size_t index) const
  {
    return _it[index];
  }

  V& at(K const& k)
  {
    return _map.at(k);
  }

  V const& at(K const& k) const
  {
    return _map.at(k);
  }

  m_iterator find(K const& k)
  {
    return _map.find(k);
  }

  m_const_iterator find(K const& k) const
  {
    return _map.find(k);
  }

  std::size_t size() const
  {
    return _it.size();
  }

  bool empty() const
  {
    return _it.empty();
  }

  Ordered_Map& clear()
  {
    _it.clear();
    _map.clear();

    return *this;
  }

  Ordered_Map& erase(K const& k)
  {
    auto it = _map.find(k);

    if (it != _map.end())
    {
      for (auto e = _it.begin(); e < _it.end(); ++e)
      {
        if ((*e) == it)
        {
          _it.erase(e);
          break;
        }
      }

      _map.erase(it);
    }

    return *this;
  }

  i_iterator begin()
  {
    return _it.begin();
  }

  i_const_iterator begin() const
  {
    return _it.begin();
  }

  i_const_iterator cbegin() const
  {
    return _it.cbegin();
  }

  i_iterator end()
  {
    return _it.end();
  }

  i_const_iterator end() const
  {
    return _it.end();
  }

  i_const_iterator cend() const
  {
    return _it.cend();
  }

  m_iterator map_begin()
  {
    return _map.begin();
  }

  m_const_iterator map_begin() const
  {
    return _map.begin();
  }

  m_const_iterator map_cbegin() const
  {
    return _map.cbegin();
  }

  m_iterator map_end()
  {
    return _map.end();
  }

  m_const_iterator map_end() const
  {
    return _map.end();
  }

  m_const_iterator map_cend() const
  {
    return _map.cend();
  }

private:

  std::unordered_map<K, V> _map;
  std::deque<m_iterator> _it;
}; // class Ordered_Map

namespace Detail
{

// prototypes
inline std::string lowercase(std::string str);
inline std::optional<std::string> extension(std::string const& path);
inline std::vector<std::string> split(std::string const& str, std::string const& delim,
  std::size_t size = std::numeric_limits<std::size_t>::max());

// string to lowercase
inline std::string lowercase(std::string str)
{
  auto const to_lower = [](char& c)
  {
    if (c >= 'A' && c <= 'Z')
    {
      c += 'a' - 'A';
    }

    return c;
  };

  for (char& c : str)
  {
    c = to_lower(c);
  }

  return str;
}

// find extension if present in a string path
inline std::optional<std::string> extension(std::string const& path)
{
  if (path.empty() || path.size() < 2)
  {
    return {};
  }

  auto const pos = path.rfind(".");

  if (pos == std::string::npos || pos == path.size() - 1)
  {
    return {};
  }

  return path.substr(pos + 1);
}

// split a string by a delimiter 'n' times
inline std::vector<std::string> split(std::string const& str,
  std::string const& delim, std::size_t times)
{
  std::vector<std::string> vtok;
  std::size_t start {0};
  auto end = str.find(delim);

  while ((times-- > 0) && (end != std::string::npos))
  {
    vtok.emplace_back(str.substr(start, end - start));
    start = end + delim.length();
    end = str.find(delim, start);
  }
  vtok.emplace_back(str.substr(start, end));

  return vtok;
}

// convert object into a string
template<typename T>
inline std::string to_string(T const& t)
{
  std::stringstream ss;
  ss << t;

  return ss.str();
}

#ifdef OB_BELLE_CONFIG_SSL_ON
// TODO switch to boost::beast::ssl_stream when it moves out of experimental
template<typename Next_Layer>
class ssl_stream : public ssl::stream_base
{
// This class (ssl_stream) is a derivative work based on Boost.Beast,
// orignal copyright below:
/*
  Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)

  Boost Software License - Version 1.0 - August 17th, 2003

  Permission is hereby granted, free of charge, to any person or organization
  obtaining a copy of the software and accompanying documentation covered by
  this license (the "Software") to use, reproduce, display, distribute,
  execute, and transmit the Software, and to prepare derivative works of the
  Software, and to permit third-parties to whom the Software is furnished to
  do so, all subject to the following:

  The copyright notices in the Software and this entire statement, including
  the above license grant, this restriction and the following disclaimer,
  must be included in all copies of the Software, in whole or in part, and
  all derivative works of the Software, unless such copies or derivative
  works are solely in the form of machine-executable object code generated by
  a source language processor.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
  SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
  FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

  using stream_type = ssl::stream<Next_Layer>;

public:

  using native_handle_type = typename stream_type::native_handle_type;
  using impl_struct = typename stream_type::impl_struct;
  using next_layer_type = typename stream_type::next_layer_type;
  using lowest_layer_type = typename stream_type::lowest_layer_type;
  using executor_type = typename stream_type::executor_type;

  ssl_stream(Next_Layer&& arg, ssl::context& ctx) :
    _ptr {std::make_unique<stream_type>(std::move(arg), ctx)}
  {
  }

  executor_type get_executor() noexcept
  {
    return _ptr->get_executor();
  }

  native_handle_type native_handle()
  {
    return _ptr->native_handle();
  }

  next_layer_type const& next_layer() const
  {
    return _ptr->next_layer();
  }

  next_layer_type& next_layer()
  {
    return _ptr->next_layer();
  }

  lowest_layer_type& lowest_layer()
  {
    return _ptr->lowest_layer();
  }

  lowest_layer_type const& lowest_layer() const
  {
    return _ptr->lowest_layer();
  }

  void set_verify_mode(ssl::verify_mode v)
  {
    _ptr->set_verify_mode(v);
  }

  void set_verify_mode(ssl::verify_mode v, error_code& ec)
  {
    _ptr->set_verify_mode(v, ec);
  }

  void set_verify_depth(int depth)
  {
    _ptr->set_verify_depth(depth);
  }

  void set_verify_depth(int depth, error_code& ec)
  {
    _ptr->set_verify_depth(depth, ec);
  }

  template<typename VerifyCallback>
  void set_verify_callback(VerifyCallback callback)
  {
    _ptr->set_verify_callback(callback);
  }

  template<typename VerifyCallback>
  void set_verify_callback(VerifyCallback callback, error_code& ec)
  {
    _ptr->set_verify_callback(callback, ec);
  }

  void handshake(handshake_type type)
  {
    _ptr->handshake(type);
  }

  void handshake(handshake_type type, error_code& ec)
  {
    _ptr->handshake(type, ec);
  }

  template<typename ConstBufferSequence>
  void handshake(handshake_type type, ConstBufferSequence const& buffers)
  {
    _ptr->handshake(type, buffers);
  }

  template<typename ConstBufferSequence>
  void handshake(handshake_type type, ConstBufferSequence const& buffers, error_code& ec)
  {
    _ptr->handshake(type, buffers, ec);
  }

  template<typename HandshakeHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(HandshakeHandler, void(error_code))
  async_handshake(handshake_type type, BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler)
  {
    return _ptr->async_handshake(type, BOOST_ASIO_MOVE_CAST(HandshakeHandler)(handler));
  }

  template<typename ConstBufferSequence, typename BufferedHandshakeHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(BufferedHandshakeHandler, void (error_code, std::size_t))
  async_handshake(handshake_type type, ConstBufferSequence const& buffers,
    BOOST_ASIO_MOVE_ARG(BufferedHandshakeHandler) handler)
  {
    return _ptr->async_handshake(type, buffers, BOOST_ASIO_MOVE_CAST(BufferedHandshakeHandler)(handler));
  }

  void shutdown()
  {
    _ptr->shutdown();
  }

  void shutdown(error_code& ec)
  {
    _ptr->shutdown(ec);
  }

  template<typename ShutdownHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ShutdownHandler, void (error_code))
  async_shutdown(BOOST_ASIO_MOVE_ARG(ShutdownHandler) handler)
  {
    return _ptr->async_shutdown(BOOST_ASIO_MOVE_CAST(ShutdownHandler)(handler));
  }

  template<typename ConstBufferSequence>
  std::size_t write_some(ConstBufferSequence const& buffers)
  {
    return _ptr->write_some(buffers);
  }

  template<typename ConstBufferSequence>
  std::size_t write_some(ConstBufferSequence const& buffers, error_code& ec)
  {
    return _ptr->write_some(buffers, ec);
  }

  template<typename ConstBufferSequence, typename WriteHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void (error_code, std::size_t))
  async_write_some(ConstBufferSequence const& buffers,
    BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
  {
    return _ptr->async_write_some(buffers, BOOST_ASIO_MOVE_CAST(WriteHandler)(handler));
  }

  template<typename MutableBufferSequence>
  std::size_t read_some(MutableBufferSequence const& buffers)
  {
    return _ptr->read_some(buffers);
  }

  template<typename MutableBufferSequence>
  std::size_t read_some(MutableBufferSequence const& buffers, error_code& ec)
  {
    return _ptr->read_some(buffers, ec);
  }

  template<typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(error_code, std::size_t))
  async_read_some(MutableBufferSequence const& buffers,
    BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
  {
    return _ptr->async_read_some(buffers, BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));
  }

  template<typename SyncStream>
  friend void teardown(websocket::role_type,
    ssl_stream<SyncStream>& stream, error_code& ec);

  template<typename AsyncStream, typename TeardownHandler>
  friend void async_teardown(websocket::role_type,
    ssl_stream<AsyncStream>& stream, TeardownHandler&& handler);

private:

  std::unique_ptr<stream_type> _ptr;
}; // class ssl_stream

template<typename SyncStream>
inline void teardown(websocket::role_type role,
  ssl_stream<SyncStream>& stream, error_code& ec)
{
  websocket::teardown(role, *stream._ptr, ec);
}

template<typename AsyncStream, typename TeardownHandler>
inline void async_teardown(websocket::role_type role,
  ssl_stream<AsyncStream>& stream, TeardownHandler&& handler)
{
  websocket::async_teardown(role, *stream._ptr, std::forward<TeardownHandler>(handler));
}
#endif // OB_BELLE_CONFIG_SSL_ON

} // namespace Detail

std::unordered_map<std::string, std::string> const mime_types
{
  {"html", "text/html"},
  {"htm", "text/html"},
  {"shtml", "text/html"},
  {"css", "text/css"},
  {"xml", "text/xml"},
  {"gif", "image/gif"},
  {"jpg", "image/jpg"},
  {"jpeg", "image/jpg"},
  {"js", "application/javascript"},
  {"atom", "application/atom+xml"},
  {"rss", "application/rss+xml"},
  {"mml", "text/mathml"},
  {"txt", "text/plain"},
  {"jad", "text/vnd.sun.j2me.app-descriptor"},
  {"wml", "text/vnd.wap.wml"},
  {"htc", "text/x-component"},
  {"png", "image/png"},
  {"tif", "image/tiff"},
  {"tiff", "image/tiff"},
  {"wbmp", "image/vnd.wap.wbmp"},
  {"ico", "image/x-icon"},
  {"jng", "image/x-jng"},
  {"bmp", "image/x-ms-bmp"},
  {"svg", "image/svg+xml"},
  {"svgz", "image/svg+xml"},
  {"webp", "image/webp"},
  {"woff", "application/font-woff"},
  {"jar", "application/java-archive"},
  {"war", "application/java-archive"},
  {"ear", "application/java-archive"},
  {"json", "application/json"},
  {"hqx", "application/mac-binhex40"},
  {"doc", "application/msword"},
  {"pdf", "application/pdf"},
  {"ps", "application/postscript"},
  {"eps", "application/postscript"},
  {"ai", "application/postscript"},
  {"rtf", "application/rtf"},
  {"m3u8", "application/vnd.apple.mpegurl"},
  {"xls", "application/vnd.ms-excel"},
  {"eot", "application/vnd.ms-fontobject"},
  {"ppt", "application/vnd.ms-powerpoint"},
  {"wmlc", "application/vnd.wap.wmlc"},
  {"kml", "application/vnd.google-earth.kml+xml"},
  {"kmz", "application/vnd.google-earth.kmz"},
  {"7z", "application/x-7z-compressed"},
  {"cco", "application/x-cocoa"},
  {"jardiff", "application/x-java-archive-diff"},
  {"jnlp", "application/x-java-jnlp-file"},
  {"run", "application/x-makeself"},
  {"pm", "application/x-perl"},
  {"pl", "application/x-perl"},
  {"pdb", "application/x-pilot"},
  {"prc", "application/x-pilot"},
  {"rar", "application/x-rar-compressed"},
  {"rpm", "application/x-redhat-package-manager"},
  {"sea", "application/x-sea"},
  {"swf", "application/x-shockwave-flash"},
  {"sit", "application/x-stuffit"},
  {"tk", "application/x-tcl"},
  {"tcl", "application/x-tcl"},
  {"crt", "application/x-x509-ca-cert"},
  {"pem", "application/x-x509-ca-cert"},
  {"der", "application/x-x509-ca-cert"},
  {"xpi", "application/x-xpinstall"},
  {"xhtml", "application/xhtml+xml"},
  {"xspf", "application/xspf+xml"},
  {"zip", "application/zip"},
  {"dll", "application/octet-stream"},
  {"exe", "application/octet-stream"},
  {"bin", "application/octet-stream"},
  {"deb", "application/octet-stream"},
  {"dmg", "application/octet-stream"},
  {"img", "application/octet-stream"},
  {"iso", "application/octet-stream"},
  {"msm", "application/octet-stream"},
  {"msp", "application/octet-stream"},
  {"msi", "application/octet-stream"},
  {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
  {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
  {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
  {"kar", "audio/midi"},
  {"midi", "audio/midi"},
  {"mid", "audio/midi"},
  {"mp3", "audio/mpeg"},
  {"ogg", "audio/ogg"},
  {"m4a", "audio/x-m4a"},
  {"ra", "audio/x-realaudio"},
  {"3gp", "video/3gpp"},
  {"3gpp", "video/3gpp"},
  {"ts", "video/mp2t"},
  {"mp4", "video/mp4"},
  {"mpg", "video/mpeg"},
  {"mpeg", "video/mpeg"},
  {"mov", "video/quicktime"},
  {"webm", "video/webm"},
  {"flv", "video/x-flv"},
  {"m4v", "video/x-m4v"},
  {"mng", "video/x-mng"},
  {"asf", "video/x-ms-asf"},
  {"asx", "video/x-ms-asf"},
  {"wmv", "video/x-ms-wmv"},
  {"avi", "video/x-msvideo"},
};

// prototypes
inline std::string mime_type(std::string const& path);

// find the mime type of a string path
inline std::string mime_type(std::string const& path)
{
  if (auto ext = Detail::extension(path))
  {
    auto const str = Detail::lowercase(*ext);

    if (mime_types.find(str) != mime_types.end())
    {
      return mime_types.at(str);
    }
  }

  return "application/octet-stream";
}

class Request : public http::request<http::string_body>
{
  using Base = http::request<http::string_body>;

public:

  using Path = std::vector<std::string>;
  using Params = std::unordered_multimap<std::string, std::string>;

  // inherit base constructors
  using http::request<http::string_body>::message;

  // default constructor
  Request() = default;

  // copy constructor
  Request(Request const&) = default;

  // move constructor
  Request(Request&&) = default;

  // copy assignment
  Request& operator=(Request const&) = default;

  // move assignment
  Request& operator=(Request&& rhs) = default;

  // default deconstructor
  ~Request() = default;

  Request&& move() noexcept
  {
    return std::move(*this);
  }

  // get the path
  Path& path()
  {
    return _path;
  }

  // get the query parameters
  Params& params()
  {
    return _params;
  }

  // serialize path and query parameters to the target
  void params_serialize()
  {
    std::string path {target().to_string()};

    _path.clear();
    _path.emplace_back(path);

    if (! _params.empty())
    {
      path += "?";
      auto it = _params.begin();
      for (; it != _params.end(); ++it)
      {
        path += url_encode(it->first) + "=" + url_encode(it->second) + "&";
      }
      path.pop_back();
    }

    target(path);
  }

  // parse the query parameters from the target
  void params_parse()
  {
    std::string path {target().to_string()};

    // separate the query params
    auto params = Detail::split(path, "?", 1);

    // set params
    if (params.size() == 2)
    {
      auto kv = Detail::split(params.at(1), "&");

      for (auto const& e : kv)
      {
        if (e.empty())
        {
          continue;
        }

        auto k_v = Detail::split(e, "=", 1);

        if (k_v.size() == 1)
        {
          _params.emplace(url_decode(e), "");
        }

        else if (k_v.size() == 2)
        {
          _params.emplace(url_decode(k_v.at(0)), url_decode(k_v.at(1)));
        }

        continue;
      }
    }
  }

private:

  std::string hex_encode(char const c)
  {
    char s[3];

    if (c & 0x80)
    {
      std::snprintf(&s[0], 3, "%02X",
        static_cast<unsigned int>(c & 0xff)
      );
    }
    else
    {
      std::snprintf(&s[0], 3, "%02X",
        static_cast<unsigned int>(c)
      );
    }

    return std::string(s);
  }

  char hex_decode(std::string const& s)
  {
    unsigned int n;

    std::sscanf(s.data(), "%x", &n);

    return static_cast<char>(n);
  }

  std::string url_encode(std::string const& str)
  {
    std::string res;
    res.reserve(str.size());

    for (auto const& e : str)
    {
      if (e == ' ')
      {
        res += "+";
      }
      else if (std::isalnum(static_cast<unsigned char>(e)) ||
        e == '-' || e == '_' || e == '.' || e == '~')
      {
        res += e;
      }
      else
      {
        res += "%" + hex_encode(e);
      }
    }

    return res;
  }

  std::string url_decode(std::string const& str)
  {
    std::string res;
    res.reserve(str.size());

    for (std::size_t i = 0; i < str.size(); ++i)
    {
      if (str[i] == '+')
      {
        res += " ";
      }
      else if (str[i] == '%' && i + 2 < str.size() &&
        std::isxdigit(static_cast<unsigned char>(str[i + 1])) &&
        std::isxdigit(static_cast<unsigned char>(str[i + 2])))
      {
        res += hex_decode(str.substr(i + 1, 2));
        i += 2;
      }
      else
      {
        res += str[i];
      }
    }

    return res;
  }

  Path _path {};
  Params _params {};
}; // Request

// store a type erased websocket
struct Websocket_Session
{
  // default deconstructor
  virtual ~Websocket_Session() = default;

  // send a message
  virtual void send(std::string const&&) = 0;
}; // struct Websocket_Session

#ifdef OB_BELLE_CONFIG_SERVER_ON
class Server
{
public:

  // NOTE Channel implementation is NOT thread safe
  class Channel
  {
  public:

    Channel()
    {
    }

    void join(Websocket_Session& socket_)
    {
      _sockets.insert(&socket_);
    }

    void leave(Websocket_Session& socket_)
    {
      _sockets.erase(&socket_);
    }

    void broadcast(std::string const&& str_) const
    {
      for(auto const e : _sockets)
      {
        e->send(std::move(str_));
      }
    }

    std::size_t size() const
    {
      return _sockets.size();
    }

  private:

    std::unordered_set<Websocket_Session*> _sockets;
  }; // class Channel

  // NOTE Channels implementation is NOT thread safe
  using Channels = std::unordered_map<std::string, Channel>;

  template<typename Body>
  struct Http_Ctx_Basic
  {
    Request req {};
    http::response<Body> res {};
    std::shared_ptr<void> data {nullptr};
  }; // class Http_Ctx_Basic

  using Http_Ctx = Http_Ctx_Basic<http::string_body>;

  class Websocket_Ctx
  {
  public:

    Websocket_Ctx(Websocket_Session& socket_, Request&& req_,
      Channels& channels_) :
      socket {&socket_},
      req {std::move(req_)},
      channels {channels_}
    {
    }

    ~Websocket_Ctx()
    {
    }

    void send(std::string const&& str_) const
    {
      socket->send(std::move(str_));
    }

    void broadcast(std::string const&& str_) const
    {
      for (auto const& e : channels)
      {
        e.second.broadcast(std::move(str_));
      }
    }

    Websocket_Session* socket;
    Request req;
    Channels& channels;
    std::string msg {};
    std::shared_ptr<void> data {nullptr};
  }; // class Websocket_Ctx

  // callbacks
  using fn_on_signal = std::function<void(error_code, int)>;
  using fn_on_http = std::function<void(Http_Ctx&)>;
  using fn_on_websocket = std::function<void(Websocket_Ctx&)>;

  struct fns_on_websocket
  {
    fns_on_websocket(fn_on_websocket const& begin_,
      fn_on_websocket const& data_, fn_on_websocket const& end_) :
      begin {begin_},
      data {data_},
      end {end_}
    {
    }

    fn_on_websocket begin {};
    fn_on_websocket data {};
    fn_on_websocket end {};
  }; // struct fns_on_websocket

  // aliases
  using Http_Routes =
    Ordered_Map<std::string, std::unordered_map<int, fn_on_http>>;

  using Websocket_Routes =
    std::vector<std::pair<std::string, fns_on_websocket>>;

private:

  struct Attr
  {
#ifdef OB_BELLE_CONFIG_SSL_ON
    // use ssl
    bool ssl {false};

    // ssl context
    ssl::context ssl_context {ssl::context::tlsv12_server};
#endif // OB_BELLE_CONFIG_SSL_ON

    // the public directory for serving static files
    std::string public_dir {};

    // default index filename for the public directory
    std::string index_file {"index.html"};

    // socket timeout
    std::chrono::seconds timeout {10};

    // serve static files from public directory
    bool http_static {true};

    // serve dynamic content
    bool http_dynamic {true};

    // upgrade http to websocket connection
    bool websocket {true};

    // default http headers
    Headers http_headers {};

    // http routes
    Http_Routes http_routes {};

    // websocket routes
    Websocket_Routes websocket_routes {};

    // callbacks for http
    fn_on_http on_http_error {};
    fn_on_http on_http_connect {};
    fn_on_http on_http_disconnect {};

    // callbacks for websocket
    fn_on_websocket on_websocket_error {};
    fn_on_websocket on_websocket_connect {};
    fn_on_websocket on_websocket_disconnect {};

    // websocket channels
    Channels channels {};
  }; // struct Attr

  template<typename Derived>
  class Websocket_Base : public Websocket_Session
  {
    Derived& derived()
    {
      return static_cast<Derived&>(*this);
    }

  public:

    Websocket_Base(net::io_context& io_, std::shared_ptr<Attr> const attr_,
      Request&& req_, fns_on_websocket const& on_websocket_) :
      _attr {attr_},
      _ctx {static_cast<Derived&>(*this), std::move(req_), _attr->channels},
      _on_websocket {on_websocket_},
      _strand {io_.get_executor()}
    {
    }

    ~Websocket_Base()
    {
      // leave channel
      _attr->channels.at(_ctx.req.path().at(0)).leave(derived());

      if (_on_websocket.end)
      {
        try
        {
          // run user function
          _on_websocket.end(_ctx);
        }
        catch (...)
        {
          this->handle_error();
        }
      }

      if (_attr->on_websocket_disconnect)
      {
        try
        {
          // run user function
          _attr->on_websocket_disconnect(_ctx);
        }
        catch (...)
        {
          this->handle_error();
        }
      }
    }

    void send(std::string const&& str_)
    {
      auto const pstr = std::make_shared<std::string const>(std::move(str_));
      _que.emplace_back(pstr);

      if (_que.size() > 1)
      {
        return;
      }

      derived().socket().async_write(net::buffer(*_que.front()),
        [self = derived().shared_from_this()](error_code ec, std::size_t bytes)
        {
          self->on_write(ec, bytes);
        }
      );
    }

    void handle_error()
    {
      if (_attr->on_websocket_error)
      {
        try
        {
          // run user function
          _attr->on_websocket_error(_ctx);
        }
        catch (...)
        {
        }
      }
    }

    void do_accept()
    {
      derived().socket().control_callback(
        [this](websocket::frame_type type, boost::beast::string_view data)
        {
          this->on_control_callback(type, data);
        }
      );

      derived().socket().async_accept_ex(_ctx.req,
        [&](auto& res)
        {
          for (auto const& e : _attr->http_headers)
          {
            res.insert(e.name_string(), *e);
          }
        },
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec)
          {
            self->on_accept(ec);
          }
        )
      );
    }

    void on_accept(error_code ec_)
    {
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }

      // join channel
      if (_attr->channels.find(_ctx.req.path().at(0)) == _attr->channels.end())
      {
        _attr->channels[_ctx.req.path().at(0)] = Channel();
      }
      _attr->channels.at(_ctx.req.path().at(0)).join(derived());

      if (_attr->on_websocket_connect)
      {
        try
        {
          // run user function
          _attr->on_websocket_connect(_ctx);
        }
        catch (...)
        {
          this->handle_error();
        }
      }

      if (_on_websocket.begin)
      {
        try
        {
          // run user function
          _on_websocket.begin(_ctx);
        }
        catch (...)
        {
          this->handle_error();
        }
      }

      this->do_read();
    }

    void on_control_callback(websocket::frame_type type_, boost::beast::string_view data_)
    {
      boost::ignore_unused(type_, data_);
    }

    void do_read()
    {
      derived().socket().async_read(_buf,
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec, std::size_t bytes)
          {
            self->on_read(ec, bytes);
          }
        )
      );
    }

    void on_read(error_code ec_, std::size_t bytes_)
    {
      boost::ignore_unused(bytes_);

      // socket closed by the timer
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      // socket closed
      if (ec_ == websocket::error::closed)
      {
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }

      if (_on_websocket.data)
      {
        try
        {
          _ctx.msg = boost::beast::buffers_to_string(_buf.data());

          // run user function
          _on_websocket.data(_ctx);
        }
        catch (...)
        {
          handle_error();
        }
      }

      // clear the request object
      _ctx.req.clear();

      // clear the buffers
      _buf.consume(_buf.size());

      this->do_read();
    }

    void on_write(error_code ec_, std::size_t bytes_)
    {
      boost::ignore_unused(bytes_);

      // happens when the timer closes the socket
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }

      // remove sent message from the queue
      _que.pop_front();

      if (_que.empty())
      {
        return;
      }

      derived().socket().async_write(net::buffer(*_que.front()),
        [self = derived().shared_from_this()](error_code ec, std::size_t bytes)
        {
          self->on_write(ec, bytes);
        }
      );
    }

    std::shared_ptr<Attr> const _attr;
    Websocket_Ctx _ctx;
    fns_on_websocket const& _on_websocket;
    net::strand<net::io_context::executor_type> _strand;
    boost::beast::multi_buffer _buf;
    std::deque<std::shared_ptr<std::string const>> _que {};
  }; // class Websocket_Base

  class Websocket :
    public Websocket_Base<Websocket>,
    public std::enable_shared_from_this<Websocket>
  {
  public:

    Websocket(tcp::socket&& socket_, std::shared_ptr<Attr> const attr_,
      Request&& req_, fns_on_websocket const& on_websocket_) :
      Websocket_Base<Websocket> {
        static_cast<net::io_context&>(socket_.get_executor().context()), 
        attr_, std::move(req_), on_websocket_},
      _socket {std::move(socket_)}
    {
    }

    ~Websocket()
    {
    }

    websocket::stream<tcp::socket>& socket()
    {
      return _socket;
    }

    void run()
    {
      this->do_accept();
    }

    void do_timeout()
    {
      this->do_shutdown();
    }

    void do_shutdown()
    {
      _socket.async_close(websocket::close_code::normal,
        net::bind_executor(this->_strand,
          [self = this->shared_from_this()](error_code ec)
          {
            self->on_shutdown(ec);
          }
        )
      );
    }

    void on_shutdown(error_code ec_)
    {
      if (ec_)
      {
        // TODO log here
        return;
      }
    }

  private:

    websocket::stream<tcp::socket> _socket;
  }; // class Websocket

#ifdef OB_BELLE_CONFIG_SSL_ON
  class Websockets :
    public Websocket_Base<Websockets>,
    public std::enable_shared_from_this<Websockets>
  {
  public:

    Websockets(Detail::ssl_stream<tcp::socket>&& socket_, std::shared_ptr<Attr> const attr_,
      Request&& req_, fns_on_websocket const& on_websocket_) :
      Websocket_Base<Websockets> {socket_.get_executor().context(), attr_,
        std::move(req_), on_websocket_},
      _socket {std::move(socket_)}
    {
    }

    ~Websockets()
    {
    }

    websocket::stream<Detail::ssl_stream<tcp::socket>>& socket()
    {
      return _socket;
    }

    void run()
    {
      this->do_accept();
    }

    void do_timeout()
    {
      this->do_shutdown();
    }

    void do_shutdown()
    {
      _socket.async_close(websocket::close_code::normal,
        net::bind_executor(this->_strand,
          [self = this->shared_from_this()](error_code ec)
          {
            self->on_shutdown(ec);
          }
        )
      );
    }

    void on_shutdown(error_code ec_)
    {
      if (ec_)
      {
        // TODO log here
        return;
      }
    }

  private:

    websocket::stream<Detail::ssl_stream<tcp::socket>> _socket;
  }; // class Websockets
#endif // OB_BELLE_CONFIG_SSL_ON

  template<typename Derived, typename Websocket_Type>
  class Http_Base
  {
    Derived& derived()
    {
      return static_cast<Derived&>(*this);
    }

  public:

    Http_Base(net::io_context& io_, std::shared_ptr<Attr> const attr_) :
      _strand {io_.get_executor()},
      _timer {io_, (std::chrono::steady_clock::time_point::max)()},
      _attr {attr_}
    {
    }

    ~Http_Base()
    {
    }

// TODO remove shim once visual studio supports generic lambdas
#ifdef _MSC_VER
    template<typename Self, typename Res>
    static void constexpr send(Self self, Res&& res)
#else
    // generic lambda for sending different types of responses
    static auto constexpr send = [](auto self, auto&& res) -> void
#endif // _MSC_VER
    {
      using item_type = std::remove_reference_t<decltype(res)>;

      auto ptr = std::make_shared<item_type>(std::move(res));
      self->_res = ptr;

      http::async_write(self->derived().socket(), *ptr,
        net::bind_executor(self->_strand,
          [self, close = ptr->need_eof()]
          (error_code ec, std::size_t bytes)
          {
            self->on_write(ec, bytes, close);
          }
        )
      );
    };

    int serve_static()
    {
      if (! _attr->http_static || _attr->public_dir.empty())
      {
        return 404;
      }

      if ((_ctx.req.method() != http::verb::get) && (_ctx.req.method() != http::verb::head))
      {
        return 404;
      }

      std::string path {_attr->public_dir + _ctx.req.target().to_string()};

      if (path.back() == '/')
      {
        path += _attr->index_file;
      }

      error_code ec;
      http::file_body::value_type body;
      body.open(path.data(), beast::file_mode::scan, ec);

      if (ec)
      {
        return 404;
      }

      // head request
      if (_ctx.req.method() == http::verb::head)
      {
        http::response<http::empty_body> res {};
        res.base() = http::response_header<>(_attr->http_headers);
        res.version(_ctx.req.version());
        res.keep_alive(_ctx.req.keep_alive());
        res.content_length(body.size());
        res.set(Header::content_type, mime_type(path));
        send(derived().shared_from_this(), std::move(res));
        return 0;
      }

      // get request
      auto const size = body.size();
      http::response<http::file_body> res {
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(_attr->http_headers)
      };
      res.version(_ctx.req.version());
      res.keep_alive(_ctx.req.keep_alive());
      res.content_length(size);
      res.set(Header::content_type, mime_type(path));
      send(derived().shared_from_this(), std::move(res));
      return 0;
    }

    int serve_dynamic()
    {
      if (! _attr->http_dynamic || _attr->http_routes.empty())
      {
        return 404;
      }

      // regex variables
      std::smatch rx_match {};
      std::regex_constants::syntax_option_type const rx_opts {std::regex::ECMAScript};
      std::regex_constants::match_flag_type const rx_flgs {std::regex_constants::match_not_null};

      // the request path
      std::string path {_ctx.req.target().to_string()};

      // separate the query parameters
      auto params = Detail::split(path, "?", 1);
      path = params.at(0);

      // iterate over routes
      for (auto const& regex_method : _attr->http_routes)
      {
        bool method_match {false};
        auto match = (*regex_method).second.find(0);

        if (match != (*regex_method).second.end())
        {
          method_match = true;
        }
        else
        {
          match = (*regex_method).second.find(static_cast<int>(_ctx.req.method()));

          if (match != (*regex_method).second.end())
          {
            method_match = true;
          }
        }

        if (method_match)
        {
          std::regex rx_str {(*regex_method).first, rx_opts};
          if (std::regex_match(path, rx_match, rx_str, rx_flgs))
          {
            // set the path
            for (auto const& e : rx_match)
            {
              _ctx.req.path().emplace_back(e.str());
            }

            // parse target params
            _ctx.req.params_parse();

            // set callback function
            auto const& user_func = match->second;

            try
            {
              // run user function
              user_func(_ctx);

              _ctx.res.content_length(_ctx.res.body().size());
              send(derived().shared_from_this(), std::move(_ctx.res));
              return 0;
            }
            catch (int const e)
            {
              return e;
            }
            catch (unsigned int const e)
            {
              return static_cast<int>(e);
            }
            catch (Status const e)
            {
              return static_cast<int>(e);
            }
            catch (std::exception const&)
            {
              return 500;
            }
            catch (...)
            {
              return 500;
            }
          }
        }
      }

      return 404;
    }

    void serve_error(int err)
    {
      _ctx.res.result(static_cast<unsigned int>(err));

      if (_attr->on_http_error)
      {
        try
        {
          // run user function
          _attr->on_http_error(_ctx);

          _ctx.res.content_length(_ctx.res.body().size());
          send(derived().shared_from_this(), std::move(_ctx.res));
          return;
        }
        catch (int const e)
        {
          _ctx.res.result(static_cast<unsigned int>(e));
        }
        catch (unsigned int const e)
        {
          _ctx.res.result(e);
        }
        catch (Status const e)
        {
          _ctx.res.result(e);
        }
        catch (std::exception const&)
        {
          _ctx.res.result(500);
        }
        catch (...)
        {
          _ctx.res.result(500);
        }
      }

      _ctx.res.set(Header::content_type, "text/plain");
      _ctx.res.body() = "Error: " + std::to_string(_ctx.res.result_int());
      _ctx.res.content_length(_ctx.res.body().size());
      send(derived().shared_from_this(), std::move(_ctx.res));
    };

    void handle_request()
    {
      // set default response values
      _ctx.res.version(_ctx.req.version());
      _ctx.res.keep_alive(_ctx.req.keep_alive());

      if (_ctx.req.target().empty())
      {
        _ctx.req.target() = "/";
      }

      if (_ctx.req.target().at(0) != '/' ||
        _ctx.req.target().find("..") != boost::beast::string_view::npos)
      {
        this->serve_error(404);
        return;
      }

      // serve dynamic content
      auto dyna = this->serve_dynamic();
      // success
      if (dyna == 0)
      {
        return;
      }
      // error
      if (dyna != 404)
      {
        this->serve_error(dyna);
        return;
      }

      // serve static content
      auto stat = this->serve_static();
      if (stat != 0)
      {
        this->serve_error(stat);
        return;
      }
    }

    bool handle_websocket()
    {
      // the request path
      std::string path {_ctx.req.target().to_string()};

      // separate the query parameters
      auto params = Detail::split(path, "?", 1);
      path = params.at(0);

      // regex variables
      std::smatch rx_match {};
      std::regex_constants::syntax_option_type const rx_opts {std::regex::ECMAScript};
      std::regex_constants::match_flag_type const rx_flgs {std::regex_constants::match_not_null};

      // check for matching route
      for (auto const& [regex, callback] : _attr->websocket_routes)
      {
        std::regex rx_str {regex, rx_opts};

        if (std::regex_match(path, rx_match, rx_str, rx_flgs))
        {
          // set the path
          for (auto const& e : rx_match)
          {
            _ctx.req.path().emplace_back(e.str());
          }

          // parse target params
          _ctx.req.params_parse();

          // create websocket
          std::make_shared<Websocket_Type>
            (derived().socket_move(), _attr, std::move(_ctx.req), callback)
            ->run();

          return true;
        }
      }

      return false;
    }

    void cancel_timer()
    {
      // set the timer to expire immediately
      _timer.expires_at((std::chrono::steady_clock::time_point::min)());
    }

    void do_timer()
    {
      // wait on the timer
      _timer.async_wait(
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec)
          {
            self->on_timer(ec);
          }
        )
      );
    }

    void on_timer(error_code ec_ = {})
    {
      if (ec_ && ec_ != net::error::operation_aborted)
      {
        // TODO log here
        return;
      }

      // check if socket has been upgraded or closed
      if (_timer.expires_at() == (std::chrono::steady_clock::time_point::min)())
      {
        return;
      }

      // check expiry
      if (_timer.expiry() <= std::chrono::steady_clock::now())
      {
        derived().do_timeout();
        return;
      }
    }

    void do_read()
    {
      _timer.expires_after(_attr->timeout);

      _res = nullptr;
      _ctx = {};
      _ctx.res.base() = http::response_header<>(_attr->http_headers);

      http::async_read(derived().socket(), _buf, _ctx.req,
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec, std::size_t bytes)
          {
            self->on_read(ec, bytes);
          }
        )
      );
    }

    void on_read(error_code ec_, std::size_t bytes_)
    {
      boost::ignore_unused(bytes_);

      // the timer has closed the socket
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      // the connection has been closed
      if (ec_ == http::error::end_of_stream)
      {
        derived().do_shutdown();
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }

      // check for websocket upgrade
      if (websocket::is_upgrade(_ctx.req))
      {
        if (! _attr->websocket || _attr->websocket_routes.empty())
        {
          derived().do_shutdown();
          return;
        }

        // upgrade to websocket
        if (handle_websocket())
        {
          this->cancel_timer();
          return;
        }
        else
        {
          derived().do_shutdown();
          return;
        }
      }

      if (_attr->on_http_connect)
      {
        try
        {
          // run user func
          _attr->on_http_connect(_ctx);
        }
        catch (...)
        {
        }
      }

      this->handle_request();

      if (_attr->on_http_disconnect)
      {
        try
        {
          // run user func
          _attr->on_http_disconnect(_ctx);
        }
        catch (...)
        {
        }
      }
    }

    void on_write(error_code ec_, std::size_t bytes_, bool close_)
    {
      boost::ignore_unused(bytes_);

      // the timer has closed the socket
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }

      if (close_)
      {
        derived().do_shutdown();
        return;
      }

      // read another request
      this->do_read();
    }

    net::strand<net::io_context::executor_type> _strand;
    net::steady_timer _timer;
    boost::beast::flat_buffer _buf;
    std::shared_ptr<Attr> const _attr;
    Http_Ctx _ctx {};
    std::shared_ptr<void> _res {nullptr};
    bool _close {false};
  }; // class Http_Base

  class Http :
    public Http_Base<Http, Websocket>,
    public std::enable_shared_from_this<Http>
  {
  public:

    Http(tcp::socket socket_, std::shared_ptr<Attr> const attr_) :
      Http_Base<Http, Websocket> {
        static_cast<net::io_context&>(socket_.get_executor().context()), attr_},
      _socket {std::move(socket_)}
    {
    }

    ~Http()
    {
    }

    tcp::socket& socket()
    {
      return _socket;
    }

    tcp::socket&& socket_move()
    {
      return std::move(_socket);
    }

    void run()
    {
      this->do_timer();
      this->do_read();
    }

    void do_timeout()
    {
      this->do_shutdown();
    }

    void do_shutdown()
    {
      error_code ec;

      // send a tcp shutdown
      _socket.shutdown(tcp::socket::shutdown_send, ec);

      this->cancel_timer();

      if (ec)
      {
        // TODO log here
        return;
      }
    }

  private:

    tcp::socket _socket;
  }; // class Http

#ifdef OB_BELLE_CONFIG_SSL_ON
  class Https :
    public Http_Base<Https, Websockets>,
    public std::enable_shared_from_this<Https>
  {
  public:

    Https(tcp::socket&& socket_, std::shared_ptr<Attr> const attr_) :
      Http_Base<Https, Websockets> {socket_.get_executor().context(), attr_},
      _socket {std::move(socket_), attr_->ssl_context}
    {
      this->_close = true;
    }

    ~Https()
    {
    }

    Detail::ssl_stream<tcp::socket>& socket()
    {
      return _socket;
    }

    Detail::ssl_stream<tcp::socket>&& socket_move()
    {
      return std::move(_socket);
    }

    void run()
    {
      this->do_timer();
      this->do_handshake();
    }

    void do_timeout()
    {
      // timed out on handshake or shutdown
      if (this->_close)
      {
        return;
      }

      // reset the timer
      this->_timer.expires_at((std::chrono::steady_clock::time_point::max)());
      this->do_timer();

      this->do_shutdown();
    }

    void do_handshake()
    {
      this->_timer.expires_after(this->_attr->timeout);

      _socket.async_handshake(ssl::stream_base::server,
        net::bind_executor(this->_strand,
          [self = this->shared_from_this()](error_code ec)
          {
            self->on_handshake(ec);
          }
        )
      );
    }

    void on_handshake(error_code ec_)
    {
      // the timer has closed the socket
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }

      this->_close = false;

      this->do_read();
    }

    void do_shutdown()
    {
      this->_timer.expires_after(this->_attr->timeout);

      this->_close = true;

      // shutdown the socket
      _socket.async_shutdown(
        net::bind_executor(this->_strand,
          [self = this->shared_from_this()](error_code ec)
          {
            self->on_shutdown(ec);
          }
        )
      );
    }

    void on_shutdown(error_code ec_)
    {
      this->cancel_timer();

      // the timer has closed the socket
      if (ec_ == net::error::operation_aborted)
      {
        return;
      }

      if (ec_)
      {
        // TODO log here
        return;
      }
    }

  private:

    Detail::ssl_stream<tcp::socket> _socket;
  }; // class Https
#endif // OB_BELLE_CONFIG_SSL_ON

  template<typename Session>
  class Listener : public std::enable_shared_from_this<Listener<Session>>
  {
  public:
  unsigned short port = 0;

    Listener(net::io_context& io_, tcp::endpoint endpoint_, std::shared_ptr<Attr> const attr_) :
      _acceptor {io_},
      _socket {io_},
      _attr {attr_}
    {
      error_code ec;

      // open the acceptor
      _acceptor.open(endpoint_.protocol(), ec);
      if (ec)
      {
        // TODO log here
        return;
      }

      // allow address reuse
      _acceptor.set_option(net::socket_base::reuse_address(true), ec);
      if (ec)
      {
        // TODO log here
        return;
      }

      // bind to the server address
      _acceptor.bind(endpoint_, ec);
      port = _acceptor.local_endpoint().port();
      if (ec)
      {
        // TODO log here
        return;
      }

      // start listening for connections
      _acceptor.listen(net::socket_base::max_listen_connections, ec);

      if (ec)
      {
        // TODO log here
        return;
      }
    }

    void run()
    {
      if (! _acceptor.is_open())
      {
        // TODO log here
        return;
      }

      do_accept();
    }

  private:

    void do_accept()
    {
      _acceptor.async_accept(_socket,
        [self = this->shared_from_this()](error_code ec)
        {
          self->on_accept(ec);
        }
      );
    }

    void on_accept(error_code ec_)
    {
      if (ec_)
      {
        // TODO log here
      }
      else
      {
        // create an Http obj and run it
        std::make_shared<Session>(std::move(_socket), _attr)->run();
      }

      // accept another connection
      do_accept();
    }

  private:

    tcp::acceptor _acceptor;
    tcp::socket _socket;
    std::shared_ptr<Attr> const _attr;
  }; // class Listener

public:

  // default constructor
  Server()
  {
  }

  // constructor with address and port
  Server(std::string address_, unsigned short port_) :
    _address {address_},
    _port {port_}
  {
  }

#ifdef OB_BELLE_CONFIG_SSL_ON
  // constructor with address, port, and ssl
  Server(std::string address_, unsigned short port_, bool ssl_) :
    _address {address_},
    _port {port_}
  {
    _attr->ssl = true;
  }
#endif // OB_BELLE_CONFIG_SSL_ON

  // destructor
  ~Server()
  {
  }

  // set the listening address
  Server& address(std::string address_)
  {
    _address = address_;

    return *this;
  }

  // get the listening address
  std::string address()
  {
    return _address;
  }

  // set the listening port
  Server& port(unsigned short port_)
  {
    _port = port_;

    return *this;
  }

  // get the listening port
  unsigned short port()
  {
    return _port;
  }

  // set the public directory for serving static files
  Server& public_dir(std::string public_dir_)
  {
    if (! public_dir_.empty() && public_dir_.back() == '/')
    {
      public_dir_.pop_back();
    }

    if (public_dir_.empty())
    {
      public_dir_ = ".";
    }

    _attr->public_dir = public_dir_;

    return *this;
  }

  // get the public directory for serving static files
  std::string public_dir()
  {
    return _attr->public_dir;
  }

  // set the default index filename
  Server& index_file(std::string index_file_)
  {
    if (index_file_.empty())
    {
      _attr->index_file = "index.html";
    }
    else
    {
      _attr->index_file = index_file_;
    }

    return *this;
  }

  // get the default index filename
  std::string index_file()
  {
    return _attr->index_file;
  }

  // set the number of threads
  Server& threads(unsigned int threads_)
  {
    _threads = std::max<unsigned int>(1, threads_);

    return *this;
  }

  // get the number of threads
  unsigned int threads()
  {
    return _threads;
  }

#ifdef OB_BELLE_CONFIG_SSL_ON
  // set ssl
  Server& ssl(bool ssl_)
  {
    _attr->ssl = ssl_;

    return *this;
  }

  // get ssl
  bool ssl()
  {
    return _attr->ssl;
  }

  // get the ssl context
  ssl::context& ssl_context()
  {
    return _attr->ssl_context;
  }

  // set the ssl context
  Server& ssl_context(ssl::context&& ctx_)
  {
    _attr->ssl_context = std::move(ctx_);

    return *this;
  }
#endif // OB_BELLE_CONFIG_SSL_ON

  // set http static
  Server& http_static(bool val_)
  {
    _attr->http_static = val_;

    return *this;
  }

  // get http static
  bool http_static()
  {
    return _attr->http_static;
  }

  // set http dynamic
  Server& http_dynamic(bool val_)
  {
    _attr->http_dynamic = val_;

    return *this;
  }

  // get http dynamic
  bool http_dynamic()
  {
    return _attr->http_dynamic;
  }

  // set http static and dynamic
  Server& http(bool val_)
  {
    _attr->http_static = val_;
    _attr->http_dynamic = val_;

    return *this;
  }

  // set websocket upgrade
  Server& websocket(bool val_)
  {
    _attr->websocket = val_;

    return *this;
  }

  // get websocket upgrade
  bool websocket()
  {
    return _attr->websocket;
  }

  // set the socket timeout
  Server& timeout(std::chrono::seconds timeout_)
  {
    _attr->timeout = timeout_;

    return *this;
  }

  // get the socket timeout
  std::chrono::seconds timeout()
  {
    return _attr->timeout;
  }

  // get the io_context
  net::io_context& io()
  {
    return _io;
  }

  // set signals to capture
  Server& signals(std::vector<int> signals_)
  {
    for (auto const& e : signals_)
    {
      _signals.add(e);
    }

    return *this;
  }

  // set signal callback
  // called when a captured signal is received
  Server& on_signal(fn_on_signal on_signal_)
  {
    _on_signal = on_signal_;

    _signals.async_wait(
      [this](error_code const& ec, int sig)
      {
        this->_on_signal(ec, sig);
      }
    );

    return *this;
  }

  // set http callback matching a single method
  // called after http read
  Server& on_http(std::string route_, Method method_, fn_on_http on_http_)
  {
    if (_attr->http_routes.find(route_) == _attr->http_routes.map_end())
    {
      _attr->http_routes(route_, {{static_cast<int>(method_), on_http_}});
    }
    else
    {
      _attr->http_routes.at(route_)[static_cast<int>(method_)] = on_http_;
    }

    return *this;
  }

  // set http callback matching multiple methods
  // called after http read
  Server& on_http(std::string route_, std::vector<Method> methods_, fn_on_http on_http_)
  {
    for (auto const& e : methods_)
    {
      if (_attr->http_routes.find(route_) == _attr->http_routes.map_end())
      {
        _attr->http_routes(route_, {{static_cast<int>(e), on_http_}});
      }
      else
      {
        _attr->http_routes.at(route_)[static_cast<int>(e)] = on_http_;
      }
    }

    return *this;
  }

  // set http callback matching all methods
  // called after http read
  Server& on_http(std::string route_, fn_on_http on_http_)
  {
    if (_attr->http_routes.find(route_) == _attr->http_routes.map_end())
    {
      _attr->http_routes(route_, {{0, on_http_}});
    }
    else
    {
      _attr->http_routes.at(route_)[0] = on_http_;
    }

    return *this;
  }

  // set http error callback
  // called when an exception or error occurs
  Server& on_http_error(fn_on_http on_http_error_)
  {
    _attr->on_http_error = on_http_error_;

    return *this;
  }

  // set http connect callback
  // called at the very beginning of every http connection
  Server& on_http_connect(fn_on_http on_http_connect_)
  {
    _attr->on_http_connect = on_http_connect_;

    return *this;
  }

  // set http disconnect callback
  // called at the very end of every http connection
  Server& on_http_disconnect(fn_on_http on_http_disconnect_)
  {
    _attr->on_http_disconnect = on_http_disconnect_;

    return *this;
  }

  // set websocket data callback
  // data: called after every websocket read
  Server& on_websocket(std::string route_, fn_on_websocket data_)
  {
    _attr->websocket_routes.emplace_back(
      std::make_pair(route_, fns_on_websocket(nullptr, data_, nullptr)));

    return *this;
  }

  // set websocket begin, data, and end callbacks
  // begin: called once after connected
  // data: called after every websocket read
  // end: called once after disconnected
  Server& on_websocket(std::string route_,
    fn_on_websocket begin_, fn_on_websocket data_, fn_on_websocket end_)
  {
    _attr->websocket_routes.emplace_back(
      std::make_pair(route_, fns_on_websocket(begin_, data_, end_)));

    return *this;
  }

  // set websocket error callback
  // called when an exception or error occurs
  Server& on_websocket_error(fn_on_websocket on_websocket_error_)
  {
    _attr->on_websocket_error = on_websocket_error_;

    return *this;
  }

  // set websocket connect callback
  // called once at the very beginning after connected
  Server& on_websocket_connect(fn_on_websocket on_websocket_connect_)
  {
    _attr->on_websocket_connect = on_websocket_connect_;

    return *this;
  }

  // set websocket disconnect callback
  // called once at the very end after disconnected
  Server& on_websocket_disconnect(fn_on_websocket on_websocket_disconnect_)
  {
    _attr->on_websocket_disconnect = on_websocket_disconnect_;

    return *this;
  }

  // get http routes
  Http_Routes& http_routes()
  {
    return _attr->http_routes;
  }

  // get websocket routes
  Websocket_Routes& websocket_routes()
  {
    return _attr->websocket_routes;
  }

  // set default http headers
  Server& http_headers(Headers const& headers_)
  {
    _attr->http_headers = headers_;

    return *this;
  }

  // get default http headers
  Headers& http_headers()
  {
    return _attr->http_headers;
  }

  // get websocket channels
  Channels& channels()
  {
    return _attr->channels;
  }

  // check if address:port is already in use
  static bool available(std::string const& address_, unsigned short port_)
  {
    error_code ec;
    net::io_context io;
    tcp::acceptor acceptor(io);
    auto endpoint = tcp::endpoint(net::ip::make_address(address_), port_);

    acceptor.open(endpoint.protocol(), ec);

    if (ec)
    {
      return false;
    }

    acceptor.bind(endpoint, ec);

    if (ec)
    {
      return false;
    }

    return true;
  };

  // check if address:port is already in use
  bool available() //const
  {
    error_code ec;
    net::io_context io;
    tcp::acceptor acceptor(io);
    auto endpoint = tcp::endpoint(net::ip::make_address(_address), _port);

    acceptor.open(endpoint.protocol(), ec);

    if (ec)
    {
      return false;
    }

    acceptor.bind(endpoint, ec);

    if (ec)
    {
      return false;
    }

    _port = acceptor.local_endpoint().port();

    return true;
  };

  // start the server
  void listen(std::string address_ = "", unsigned short port_ = 0)
  {
    // set the listening address
    if (! address_.empty())
    {
      _address = address_;
    }

    // set the listening port
    if (port_ != 0)
    {
      _port = port_;
    }

    // set default server header value if not present
    if (_attr->http_headers.find(Header::server) == _attr->http_headers.end())
    {
      _attr->http_headers.set(Header::server, "Belle");
    }

    // websocket channels are not threadsafe, limit to 1 thread
    if (_attr->websocket && _threads > 1)
    {
      _threads = 1;
    }

    // create the listener
#ifdef OB_BELLE_CONFIG_SSL_ON
    if (_attr->ssl)
    {
      // use https
      std::make_shared<Listener<Https>>
        (_io, tcp::endpoint(net::ip::make_address(_address), _port), _attr)
        ->run();
    }
    else
#endif // OB_BELLE_CONFIG_SSL_ON
    {
      // use http
      std::make_shared<Listener<Http>>
        (_io, tcp::endpoint(net::ip::make_address(_address), _port), _attr)->run();
    }

    // thread pool
    std::vector<std::thread> io_threads;

    // create and start threads if needed
    if (_threads > 1)
    {
      io_threads.reserve(static_cast<std::size_t>(_threads) - 1);

      for (unsigned int i = 1; i < _threads; ++i)
      {
        io_threads.emplace_back(
          [this]()
          {
            // run the io context on the new thread
            this->_io.run();
          }
        );
      }
    }

    // run the io context on the current thread
    _io.run();

    // wait on threads to return
    for (auto& t : io_threads)
    {
      t.join();
    }
  }

private:

  // hold the server attributes shared by each socket connection
  std::shared_ptr<Attr> const _attr {std::make_shared<Attr>()};

  // the address to listen on
  std::string _address {"127.0.0.1"};

  // the port to listen on
  unsigned short _port {8080};

  // the number of threads to run on
  unsigned int _threads {1};

  // the io context
  net::io_context _io {};

  // signals
  net::signal_set _signals {_io};

  // callback for signals
  fn_on_signal _on_signal {};
}; // class Server
#endif // OB_BELLE_CONFIG_SERVER_ON

#ifdef OB_BELLE_CONFIG_CLIENT_ON
class Client
{
public:

  struct Http_Ctx
  {
    // http request
    Request* req {nullptr};

    // http response
    http::response<http::string_body> res {};
  }; // struct Http_Ctx

  struct Error_Ctx
  {
    // error code
    error_code const& ec;
  }; // struct Error_Ctx

  // callbacks
  using fn_on_http = std::function<void(Http_Ctx&)>;
  using fn_on_http_error = std::function<void(Error_Ctx&)>;

  struct Req_Ctx
  {
    // http request object
    Request req {};

    // http callback
    fn_on_http on_http {};
  }; // struct Req_Ctx

  struct Attr
  {
#ifdef OB_BELLE_CONFIG_SSL_ON
    // use ssl
    bool ssl {false};

    // ssl context
    ssl::context ssl_context {ssl::context::tlsv12_client};
#endif // OB_BELLE_CONFIG_SSL_ON

    // socket timeout
    std::chrono::seconds timeout {10};

    // address to connect to
    std::string address {"127.0.0.1"};

    // port to connect to
    unsigned short port {8080};

    // http request queue
    std::deque<Req_Ctx> que;

    // http error callback
    fn_on_http_error on_http_error {};
  }; // struct Attr

  template<typename Derived>
  class Http_Base
  {
    Derived& derived()
    {
      return static_cast<Derived&>(*this);
    }

  public:

    Http_Base(net::io_context& io_, std::shared_ptr<Attr> const attr_) :
      _resolver {io_},
      _strand {io_.get_executor()},
      _timer {io_, (std::chrono::steady_clock::time_point::max)()},
      _attr {attr_}
    {
    }

    ~Http_Base()
    {
    }

    void cancel_timer()
    {
      // set the timer to expire immediately
      _timer.expires_at((std::chrono::steady_clock::time_point::min)());
    }

    void do_timer()
    {
      // wait on the timer
      _timer.async_wait(
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec)
          {
            self->on_timer(ec);
          }
        )
      );
    }

    void on_timer(error_code ec_)
    {
      if (ec_ && ec_ != net::error::operation_aborted)
      {
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      // check if socket has been closed
      if (_timer.expires_at() == (std::chrono::steady_clock::time_point::min)())
      {
        return;
      }

      // check expiry
      if (_timer.expiry() <= std::chrono::steady_clock::now())
      {
        derived().do_close();

        return;
      }

      if (_close)
      {
        return;
      }
    }

    void do_resolve()
    {
      _timer.expires_after(_attr->timeout);

      // domain name server lookup
      _resolver.async_resolve(_attr->address,
        Detail::to_string(_attr->port),
        net::bind_executor(_strand,
          [self = derived().shared_from_this()]
          (error_code ec, tcp::resolver::results_type results)
          {
            self->on_resolve(ec, results);
          }
        )
      );
    }

    void on_resolve(error_code ec_, tcp::resolver::results_type results_)
    {
      if (ec_)
      {
        cancel_timer();
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      // connect to the endpoint
      net::async_connect(derived().socket().lowest_layer(),
        results_.begin(), results_.end(),
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec, auto)
          {
            self->derived().on_connect(ec);
          }
        )
      );
    }

    void prepare_request()
    {
      _ctx = {};
      _ctx.req = &_attr->que.front().req;

      // serialize target and params
      _ctx.req->params_serialize();

      // set default user-agent header value if not present
      if (_ctx.req->find(Header::user_agent) == _ctx.req->end())
      {
        _ctx.req->set(Header::user_agent, "Belle");
      }

      // set default host header value if not present
      if (_ctx.req->find(Header::host) == _ctx.req->end())
      {
        _ctx.req->set(Header::host, _attr->address);
      }

      // set connection close if last request in the queue
      if (_attr->que.size() == 1)
      {
        _ctx.req->keep_alive(false);
      }

      // prepare the payload
      _ctx.req->prepare_payload();
    }

    void do_write()
    {
      prepare_request();

      _timer.expires_after(_attr->timeout);

      // Send the HTTP request
      http::async_write(derived().socket(), *_ctx.req,
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec, std::size_t bytes)
          {
            self->on_write(ec, bytes);
          }
        )
      );
    }

    void on_write(error_code ec_, std::size_t bytes_)
    {
      boost::ignore_unused(bytes_);

      if (ec_)
      {
        cancel_timer();
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      do_read();
    }

    void do_read()
    {
      // Receive the HTTP response
      http::async_read(derived().socket(), _buf, _ctx.res,
        net::bind_executor(_strand,
          [self = derived().shared_from_this()](error_code ec, std::size_t bytes)
          {
            self->on_read(ec, bytes);
          }
        )
      );
    }

    void on_read(error_code ec_, std::size_t bytes_)
    {
      boost::ignore_unused(bytes_);

      if (ec_)
      {
        cancel_timer();
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      // run user function
      _attr->que.front().on_http(_ctx);

      // remove request from queue
      _attr->que.pop_front();

      if (_attr->que.empty())
      {
        derived().do_close();
      }
      else
      {
        do_write();
      }
    }

    tcp::resolver _resolver;
    net::strand<net::io_context::executor_type> _strand;
    net::steady_timer _timer;
    std::shared_ptr<Attr> const _attr;
    Http_Ctx _ctx {};
    beast::flat_buffer _buf {};
    bool _close {false};
  }; // class Http_Base

  class Http :
    public Http_Base<Http>,
    public std::enable_shared_from_this<Http>
  {
  public:
    Http(net::io_context& io_, std::shared_ptr<Attr> attr_) :
      Http_Base<Http>(io_, attr_),
      _socket {io_}
    {
    }

    ~Http()
    {
    }

    tcp::socket& socket()
    {
      return _socket;
    }

    tcp::socket&& socket_move()
    {
      return std::move(_socket);
    }

    void run()
    {
      do_timer();
      do_resolve();
    }

    void on_connect(error_code ec_)
    {
      if (ec_)
      {
        cancel_timer();
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      do_write();
    }

    void do_close()
    {
      error_code ec;

      // shutdown the socket
      _socket.shutdown(tcp::socket::shutdown_both, ec);
      _socket.close(ec);

      // ignore not_connected error
      if (ec && ec != boost::system::errc::not_connected)
      {
        cancel_timer();
        Error_Ctx err {ec};
        _attr->on_http_error(err);
        return;
      }

      // the connection is now closed
    }


  private:

    tcp::socket _socket;
  }; // class Http

#ifdef OB_BELLE_CONFIG_SSL_ON
  class Https :
    public Http_Base<Https>,
    public std::enable_shared_from_this<Https>
  {
  public:
    Https(net::io_context& io_, std::shared_ptr<Attr> attr_) :
      Http_Base<Https>(io_, attr_),
      _socket {std::move(tcp::socket(io_)), attr_->ssl_context}
    {
      _close = true;
    }

    ~Https()
    {
    }

    Detail::ssl_stream<tcp::socket>& socket()
    {
      return _socket;
    }

    Detail::ssl_stream<tcp::socket>&& socket_move()
    {
      return std::move(_socket);
    }

    void run()
    {
      // start the timer
      do_timer();

      // set server name indication
      // use SSL_ctrl instead of SSL_set_tlsext_host_name macro
      // to avoid old style C cast to char*
      // if (! SSL_set_tlsext_host_name(_socket.native_handle(), _attr->address.data()))
      if (! SSL_ctrl(_socket.native_handle(), SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, _attr->address.data()))
      {
        error_code ec
        {
          static_cast<int>(ERR_get_error()),
          net::error::get_ssl_category()
        };

        cancel_timer();
        Error_Ctx err {ec};
        _attr->on_http_error(err);
        return;
      }

      do_resolve();
    }

    void on_connect(error_code ec_)
    {
      if (ec_)
      {
        cancel_timer();
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      do_handshake();
    }

    void do_handshake()
    {
      // perform the ssl handshake
      _socket.async_handshake(ssl::stream_base::client,
        net::bind_executor(_strand,
          [self = this->shared_from_this()](error_code ec)
          {
            self->on_handshake(ec);
          }
        )
      );
    }

    void on_handshake(error_code ec_)
    {
      if (ec_)
      {
        cancel_timer();
        Error_Ctx err {ec_};
        _attr->on_http_error(err);
        return;
      }

      _close = false;

      do_write();
    }

    void do_close()
    {
      if (_close)
      {
        return;
      }

      _close = true;

      // shutdown the socket
      _socket.async_shutdown(
        net::bind_executor(_strand,
          [self = this->shared_from_this()](error_code ec)
          {
            self->on_shutdown(ec);
          }
        )
      );
    }

    void on_shutdown(error_code ec_)
    {
      cancel_timer();

      // ignore eof error
      if (ec_ == net::error::eof)
      {
        ec_.assign(0, ec_.category());
      }

      // ignore not_connected error
      if (ec_ && ec_ != boost::system::errc::not_connected)
      {
        return;
      }

      // close the socket
      _socket.next_layer().close(ec_);

      // ignore not_connected error
      if (ec_ && ec_ != boost::system::errc::not_connected)
      {
        return;
      }

      // the connection is now closed
    }

  private:

    Detail::ssl_stream<tcp::socket> _socket;
  }; // class Https
#endif // OB_BELLE_CONFIG_SSL_ON

  // default constructor
  Client()
  {
  }

  // constructor with address and port
  Client(std::string address_, unsigned short port_)
  {
    _attr->address = address_;
    _attr->port = port_;
  }

#ifdef OB_BELLE_CONFIG_SSL_ON
  // constructor with address, port, and ssl
  Client(std::string address_, unsigned short port_, bool ssl_)
  {
    _attr->address = address_;
    _attr->port = port_;
    _attr->ssl = ssl_;
  }
#endif // OB_BELLE_CONFIG_SSL_ON

  // destructor
  ~Client()
  {
  }

  // set the address to connect to
  Client& address(std::string address_)
  {
    _attr->address = address_;

    return *this;
  }

  // get the address to connect to
  std::string address()
  {
    return _attr->address;
  }

  // set the port to connect to
  Client& port(unsigned short port_)
  {
    _attr->port = port_;

    return *this;
  }

  // get the port to connect to
  unsigned short port()
  {
    return _attr->port;
  }

  // set the socket timeout
  Client& timeout(std::chrono::seconds timeout_)
  {
    _attr->timeout = timeout_;

    return *this;
  }

  // get the socket timeout
  std::chrono::seconds timeout()
  {
    return _attr->timeout;
  }

  // set the max timeout
  Client& timeout_max(std::chrono::milliseconds timeout_max_)
  {
    _timeout_max = timeout_max_;

    return *this;
  }

  // get the max timeout
  std::chrono::milliseconds timeout_max()
  {
    return _timeout_max;
  }

  // get request queue
  std::deque<Req_Ctx>& queue()
  {
    return _attr->que;
  }

  // get the io_context
  net::io_context& io()
  {
    return _io;
  }

#ifdef OB_BELLE_CONFIG_SSL_ON
  // set ssl
  Client& ssl(bool ssl_)
  {
    _attr->ssl = ssl_;

    return *this;
  }

  // get ssl
  bool ssl()
  {
    return _attr->ssl;
  }

  // get the ssl context
  ssl::context& ssl_context()
  {
    return _attr->ssl_context;
  }

  // set the ssl context
  Client& ssl_context(ssl::context&& ctx_)
  {
    _attr->ssl_context = std::move(ctx_);

    return *this;
  }
#endif // OB_BELLE_CONFIG_SSL_ON

  Client& on_http(Request const& req_, fn_on_http on_http_)
  {
    _attr->que.emplace_back(Req_Ctx());
    auto& ctx = _attr->que.back();

    ctx.req = req_;
    ctx.on_http = on_http_;

    return *this;
  }

  Client& on_http(Request&& req_, fn_on_http on_http_)
  {
    _attr->que.emplace_back(Req_Ctx());
    auto& ctx = _attr->que.back();

    ctx.req = std::move(req_);
    ctx.on_http = on_http_;

    return *this;
  }

  Client& on_http(std::string const& target_, fn_on_http on_http_)
  {
    this->on_http_impl(Method::get, target_, Request::Params(), Headers(), {}, on_http_);

    return *this;
  }

  Client& on_http(std::string const& target_, Request::Params const& params_, fn_on_http on_http_)
  {
    this->on_http_impl(Method::get, target_, params_, Headers(), {}, on_http_);

    return *this;
  }

  Client& on_http(std::string const& target_, Headers const& headers_, fn_on_http on_http_)
  {
    this->on_http_impl(Method::get, target_, Request::Params(), headers_, {}, on_http_);

    return *this;
  }

  Client& on_http(std::string const& target_, Request::Params const& params_, Headers const& headers_, fn_on_http on_http_)
  {
    this->on_http_impl(Method::get, target_, params_, headers_, {}, on_http_);

    return *this;
  }

  Client& on_http(Method method_, std::string const& target_,
    std::string const& body_, fn_on_http on_http_)
  {
    this->on_http_impl(method_, target_, Request::Params(), Headers(), body_, on_http_);

    return *this;
  }

  Client& on_http(Method method_, std::string const& target_,
    Request::Params const& params_,
    std::string const& body_, fn_on_http on_http_)
  {
    this->on_http_impl(method_, target_, params_, Headers(), body_, on_http_);

    return *this;
  }

  Client& on_http(Method method_, std::string const& target_,
    Headers const& headers_,
    std::string const& body_, fn_on_http on_http_)
  {
    this->on_http_impl(method_, target_, Request::Params(), headers_, body_, on_http_);

    return *this;
  }

  Client& on_http(Method method_, std::string const& target_,
    Request::Params const& params_, Headers const& headers_,
    std::string const& body_, fn_on_http on_http_)
  {
    this->on_http_impl(method_, target_, params_, headers_, body_, on_http_);

    return *this;
  }

  Client& on_http_error(fn_on_http_error on_http_error_)
  {
    _attr->on_http_error = on_http_error_;

    return *this;
  }

  std::size_t connect()
  {
    if (_attr->que.empty())
    {
      return 0;
    }

#ifdef OB_BELLE_CONFIG_SSL_ON
    if (_attr->ssl)
    {
      // use https
      std::make_shared<Https>(_io, _attr)->run();
    }
    else
#endif // OB_BELLE_CONFIG_SSL_ON
    {
      // use http
      std::make_shared<Http>(_io, _attr)->run();
    }

    std::size_t size_begin {_attr->que.size()};

    if (_timeout_max > std::chrono::milliseconds(0))
    {
      // run for max 'n' amount of time
      _io.run_until(std::chrono::steady_clock::now() + _timeout_max);
    }
    else
    {
      _io.run();
    }

    // reset the io_context
    _io.restart();

    std::size_t size_end {_attr->que.size()};

    return size_begin - size_end;
  }

private:

  Client& on_http_impl(Method method_, std::string const& target_,
    Request::Params const& params_, Headers const& headers_,
    std::string const& body_, fn_on_http on_http_)
  {
    _attr->que.emplace_back(Req_Ctx());
    auto& ctx = _attr->que.back();

    Request req {method_, target_, 11, body_, headers_};
    req.params() = params_;

    ctx.req = std::move(req);
    ctx.on_http = on_http_;

    return *this;
  }

  // hold the client attributes
  std::shared_ptr<Attr> const _attr {std::make_shared<Attr>()};

  // the io context
  net::io_context _io {};

  // timeout all requests after specified number of milliseconds
  std::chrono::milliseconds _timeout_max {0};
}; // class Client
#endif // OB_BELLE_CONFIG_CLIENT_ON

} // namespace OB::Belle

#endif // OB_BELLE_HH