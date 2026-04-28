#include "server/Session.h"
#include "server/HttpServer.h"

#include <iostream>
#include <utility>

namespace learnChemistry::server {

    Session::Session(tcp::socket socket, HttpServer& server)
        : socket_(std::move(socket)), server_(server) {
    }

    void Session::start() {
        doRead();
    }


    void Session::doRead() {
        buffer_.consume(buffer_.size());
        req_ = {};

        auto self = shared_from_this();
        http::async_read(socket_, buffer_, req_,
            [self](beast::error_code ec, std::size_t bytes) {
                self->onRead(ec, bytes);
            }
        );
    }

    

    void Session::onRead(beast::error_code ec, std::size_t /*bytes*/) {
        if (ec == http::error::end_of_stream) {
            return doClose();
        }

        if (ec) {
            std::cerr << "Read error: " << ec.message() << "\n";
            return;
        }

        try {
            auto res = server_.handleRequest(req_);

            // Keep protocol details consistent
            res.version(req_.version());
            res.keep_alive(req_.keep_alive());

            const bool close = res.need_eof();
            send(std::move(res));

            if (close) {
                return doClose();
            }

            // For keep-alive connections, continue reading
            doRead();
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception while handling request: " << ex.what() << "\n";

            http::response<http::string_body> res{ http::status::internal_server_error, req_.version() };
            res.set(http::field::content_type, "text/plain");
            res.body() = "Internal Server Error";
            res.prepare_payload();
            res.keep_alive(false);

            send(std::move(res));
            doClose();
        }
    }

    void Session::send(http::response<http::string_body>&& res) {
        // ✅ Store response on heap so it lives until write completes
        res_ = std::make_shared<http::response<http::string_body>>(std::move(res));

        auto self = shared_from_this();
        http::async_write(socket_, *res_,
            [self](beast::error_code ec, std::size_t bytes) {
                self->onWrite(self->res_->need_eof(), ec, bytes);
            }
        );
    }

    void Session::onWrite(bool /*close*/, beast::error_code ec, std::size_t /*bytes*/) {
        if (ec) {
            std::cerr << "Write error: " << ec.message() << "\n";
            return;
        }

        // ✅ Release response after write completes
        res_.reset();
    }

    void Session::doClose() {
        beast::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }

} // namespace learnChemistry::server