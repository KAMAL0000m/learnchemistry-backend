#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace learnChemistry::server {

    namespace http = boost::beast::http;
    namespace beast = boost::beast;

    class HttpServer;

    class Session : public std::enable_shared_from_this<Session> {
    public:
        using tcp = boost::asio::ip::tcp;

        Session(tcp::socket socket, HttpServer& server);
        void start();

    private:
        void doRead();
        void onRead(beast::error_code ec, std::size_t bytes);

        // String response write path
        void send(http::response<http::string_body>&& res);
        void onWriteString(bool close, beast::error_code ec, std::size_t bytes);

        // File response write path
        void sendFile(http::response<http::file_body>&& res);
        void onWriteFile(bool close, beast::error_code ec, std::size_t bytes);

        void doClose();

    private:
        tcp::socket socket_;
        HttpServer& server_;

        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;

        std::shared_ptr<http::response<http::string_body>> res_;
        std::shared_ptr<http::response<http::file_body>> fileRes_;
    };

} // namespace learnChemistry::server