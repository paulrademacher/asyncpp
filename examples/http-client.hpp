#pragma once

// Based on boost_lib/boost_1_55_0/doc/html/boost_asio/example/cpp03/http/client/async_client.cpp
//     by Christopher M. Kohlhoff (chris at kohlhoff dot com)

#include <sstream>
#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace http_client {

class MalformedUriException : public std::invalid_argument {
public:
  explicit MalformedUriException() : std::invalid_argument("malformed-uri") {};
};

class AsyncHttpClient {
public:
  // Creates a new client.
  AsyncHttpClient(const std::string& uri,
      const std::string& method="GET",
      const std::vector<std::string>& headers=std::vector<std::string>(),
      const std::string& body="");
  ~AsyncHttpClient();

  bool connect();

  AsyncHttpClient(const AsyncHttpClient&) = delete;
  AsyncHttpClient& operator=(const AsyncHttpClient&) = delete;

  void fetch(std::function<std::string()> callback);

private:
  boost::asio::io_service io_service_;
  std::string uri_;
  std::string method_;
  std::vector<std::string> headers_;
  std::string body_;
  std::string server_;
  std::stringstream content_;
  int port_;
  std::string path_;

  void read_content(const boost::system::error_code& err, const std::size_t bytes_transferred);

  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
};

}
