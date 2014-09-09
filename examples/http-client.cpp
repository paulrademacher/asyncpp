#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

#include "http-client.hpp"

// Based on boost_lib/boost_1_55_0/doc/html/boost_asio/example/cpp03/http/client/async_client.cpp
//     by Christopher M. Kohlhoff (chris at kohlhoff dot com)

namespace http_client {

namespace util {

std::string substr(const std::string &string, size_t pos, size_t len=0) {
  if (pos == std::string::npos) {
    return std::string("");
  }

  try {
    if (len == 0) {
      len = string.length() - pos;
    }
    return string.substr(pos, len);
  } catch (const std::out_of_range& e) {
    return std::string("");
  }
}

}

AsyncHttpClient::AsyncHttpClient(const std::string& uri,
    const std::string& method,
    const std::vector<std::string>& headers,
    const std::string& body)
  : resolver_(io_service_),
    socket_(io_service_),
    uri_(uri),
    method_(method),
    headers_(headers),
    body_(body) {

  std::cout << "----------------------------------------" << std::endl;
  std::cout << uri << std::endl;

  bool is_http = true;
  std::string uri_without_protocol;

  if (boost::algorithm::starts_with(uri_, "http://")) {
    uri_without_protocol = util::substr(uri_, 7);
  } else if (boost::algorithm::starts_with(uri_, "https://")) {
    uri_without_protocol = util::substr(uri_, 8);
    is_http = false;
  } else {
    throw MalformedUriException("Needs http:// or https://");
  }

  if (uri_without_protocol.length() == 0) {
    throw MalformedUriException("url length is zero");
  }

  size_t path_start = uri_without_protocol.find_first_of("/");
  std::string server_and_port = util::substr(uri_without_protocol, 0, path_start);
  path_ = path_start != std::string::npos ?
      util::substr(uri_without_protocol, path_start) : "/";

  if (server_and_port.length() == 0) {
    throw MalformedUriException("server and port empty");
  }

  int colon = server_and_port.find_first_of(":");
  server_ = util::substr(server_and_port, 0, colon);

  std::string port_string;
  if (colon != std::string::npos) {
    port_string = util::substr(server_and_port, colon + 1);
  }
  port_ = port_string.length() > 0 ?
      std::stoi(port_string) : (is_http ? 80 : 443);

  std::cout << "is_http: " << is_http << std::endl;
  std::cout << "server: " << server_ << std::endl;
  std::cout << "port: " << port_ << std::endl;
  std::cout << "path: " << path_ << std::endl;
}

AsyncHttpClient::~AsyncHttpClient() {
}

void AsyncHttpClient::fetch(std::function<std::string()> callback) {
  // Form the request.  We specift the "Connection: close" header so that the
  // server will close the socket after transmitting the response.  This will
  // allow us to treat all data up until the EOF as the content.

  std::ostream request_stream(&request_);  // Init the member boost::asio::streambuf.
  request_stream << "GET " << path_ << " HTTP/1.0\r\n";
  request_stream << "Host: " << server_ << "\r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";

  std::cout << "GET " << path_ << " HTTP 1.0\r\n";
  std::cout << "Host: " << server_ << "\r\n";
  std::cout << "Accept: */*\r\n";
  std::cout << "Connection: close\r\n\r\n";

  async::TaskVector<int> steps {
    /*************************************************************
     * Step 1: Resolve the URL.
     */
    [this](async::TaskCallback<int> callback) {
      // Start an asynchronous resolve to translat ethe server and service names
      // into a list of endpoints.
      boost::asio::ip::tcp::resolver::query query(server_,
          boost::lexical_cast<std::string>(port_)); // TODO: handle port number.
      resolver_.async_resolve(query,
          [=](const boost::system::error_code& err,
              boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
            endpoint_iterator_ = endpoint_iterator;

            if (err) {
              callback(async::FAIL, 1);
            } else {
              std::cout << "RESOLVED" << std::endl;
              callback(async::OK, 1);
            }
          });
    },

    /*************************************************************
     * Step 2: Connect to a valid endpoint that URL resolves to.
     */
    [this](async::TaskCallback<int> callback) {
      // Attempt a connection to each endpoint in the list until we
      // successfully establish a connection.

      boost::asio::async_connect(socket_, endpoint_iterator_,
          [=](const boost::system::error_code& err,
              boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
            if (err) {
              callback(async::FAIL, 2);
            } else {
              // The connection was successful.  Send the request in next step.
              std::cout << "CONNECTED" << " " << err << std::endl;
              callback(async::OK, 2);
            }
          });
    },

    /*************************************************************
     * Step 3: Write the URL request.
     */
    [this](async::TaskCallback<int> callback) {
      boost::asio::async_write(socket_, request_,
          [=](const boost::system::error_code& err,
              const std::size_t bytes_transferred) {
            if (err) {
              callback(async::FAIL, 3);
            } else {
              std::cout << "WRITING REQUEST" << std::endl;
              callback(async::OK, 3);
            }
          });
    },

    /************************************************************
     * Step 4: Read status line.
     */
    [this](async::TaskCallback<int> callback) {
      // Read the response status line.  The response_ streambuf will
      // automatically grow to accommodate the entire line.  The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      boost::asio::async_read_until(socket_, response_, "\r\n",
          [=](const boost::system::error_code& err,
              const std::size_t byes_transferred) {
            std::cout << "READING STATUS LINE.  err: " << err << std::endl;

            if (err) {
              callback(async::FAIL, 4);
            } else {
              std::cout << "READ STATUS LINE" << std::endl;
              callback(async::OK, 4);
            }
          });
    },

    /************************************************************
     * Step 5: Parse status line.
     */
    [this](async::TaskCallback<int> callback) {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);

      std::cout << "http_version: " << http_version << std::endl;
      std::cout << "status_code: " << status_code << std::endl;
      std::cout << "status_message: " << status_message << std::endl;

      if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response" << std::endl;
        // TODO: Handle error.
        callback(async::FAIL, 51);
        return;
      }

      if (status_code != 200) {
        std::cout << "Response returned with status code " << status_code << std::endl;
        callback(async::FAIL, 52);
        return;
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
          [=](const boost::system::error_code& err,
              const std::size_t byes_transferred) {
            std::cout << "Reading header" << std::endl;

            if (err) {
              callback(async::FAIL, 53);
            } else {
              callback(async::OK, 5);
            }
          });
    },

    /************************************************************
     * Step 6: Parse headers.
     */
    [this](async::TaskCallback<int> final_callback) {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r") {
        std::cout << "Header: " << header << std::endl;
      }
      std::cout << std::endl;

      // Read data until EOF.
      async::forever([this, final_callback](async::ErrorCodeCallback keep_reading_callback) {
            boost::asio::async_read(socket_, response_,
                boost::asio::transfer_at_least(1),
                [this, final_callback, keep_reading_callback](const boost::system::error_code& err,
                    const std::size_t bytes_transferred) {

                  if (response_.size() > 0) {
                    content_ << &response_;
                  }

                  if (err) {
                    keep_reading_callback(async::STOP);

                    // Stop the main execution sequence.
                    final_callback(err == boost::asio::error::eof ? async::OK : async::FAIL, 6);
                  } else {
                    keep_reading_callback(async::OK);
                  }
                });  // async_read()
          });  // forever()
    }
  };

  async::series<int>(steps,
      [this](async::ErrorCode error, std::vector<int> results) {
        std::cout << "ERROR CODE: " << error << std::endl << std::endl;

        // Write all of the data that has been read so far.
        std::cout << content_.str() << std::endl;
      });

  io_service_.run();
}

}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "USAGE: " << argv[0] << " URL" << std::endl;
    return 1;
  }

  http_client::AsyncHttpClient client(argv[1], "GET");
  client.fetch(nullptr);

  return 0;
}
