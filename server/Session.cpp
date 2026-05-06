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
            // Normalize target (strip query + trailing slash)
            std::string target = std::string(req_.target());
            if (auto q = target.find('?'); q != std::string::npos) target.resize(q);
            if (target.size() > 1 && target.back() == '/') target.pop_back();

            // ============================================
            // ✅ File download endpoint (/v1/download/:id)
            //    - OPTIONS => handleRequest (string, CORS)
            //    - GET     => handleDownload (file stream)
            // ============================================
            if (target.rfind("/v1/download", 0) == 0) {

                // Preflight must be handled as string response
                if (req_.method() == http::verb::options) {
                    auto res = server_.handleRequest(req_);
                    res.version(req_.version());
                    res.keep_alive(req_.keep_alive());
                    send(std::move(res));
                    return;
                }

                auto fres = server_.handleDownload(req_);
                fres.version(req_.version());
                fres.keep_alive(req_.keep_alive());
                sendFile(std::move(fres));
                return;
            }

            // ============================================
            // ✅ Thumbnail endpoint (/v1/thumb/:id) (public)
            //    - OPTIONS => handleRequest (string, CORS)
            //    - GET     => handleThumb (file stream)
            // ============================================
            if (target.rfind("/v1/thumb", 0) == 0) {

                // Preflight must be handled as string response
                if (req_.method() == http::verb::options) {
                    auto res = server_.handleRequest(req_);
                    res.version(req_.version());
                    res.keep_alive(req_.keep_alive());
                    send(std::move(res));
                    return;
                }

                auto fres = server_.handleThumb(req_);
                fres.version(req_.version());
                fres.keep_alive(req_.keep_alive());
                sendFile(std::move(fres));
                return;
            }

            // ============================================
            // Default JSON APIs (string response)
            // ============================================
            auto res = server_.handleRequest(req_);
            res.version(req_.version());
            res.keep_alive(req_.keep_alive());
            send(std::move(res));
            return;
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception while handling request: " << ex.what() << "\n";

            http::response<http::string_body> res{ http::status::internal_server_error, req_.version() };
            res.set(http::field::content_type, "text/plain");
            res.body() = "Internal Server Error";
            res.prepare_payload();
            res.keep_alive(false);

            send(std::move(res));
            return;
        }
    }

    // ---------------------------
    // String response write path
    // ---------------------------
    void Session::send(http::response<http::string_body>&& res) {
        res_ = std::make_shared<http::response<http::string_body>>(std::move(res));

        auto self = shared_from_this();
        http::async_write(socket_, *res_,
            [self](beast::error_code ec, std::size_t bytes) {
                const bool close = self->res_ ? self->res_->need_eof() : true;
                self->onWriteString(close, ec, bytes);
            }
        );
    }

    void Session::onWriteString(bool close, beast::error_code ec, std::size_t /*bytes*/) {
        if (ec) {
            std::cerr << "Write error (string): " << ec.message() << "\n";
            doClose();
            return;
        }

        res_.reset();

        if (close) {
            return doClose();
        }

        doRead();
    }

    // ---------------------------
    // File response write path
    // ---------------------------
    void Session::sendFile(http::response<http::file_body>&& res) {
        fileRes_ = std::make_shared<http::response<http::file_body>>(std::move(res));

        auto self = shared_from_this();
        http::async_write(socket_, *fileRes_,
            [self](beast::error_code ec, std::size_t bytes) {
                const bool close = self->fileRes_ ? self->fileRes_->need_eof() : true;
                self->onWriteFile(close, ec, bytes);
            }
        );
    }

    void Session::onWriteFile(bool close, beast::error_code ec, std::size_t /*bytes*/) {
        if (ec) {
            std::cerr << "Write error (file): " << ec.message() << "\n";
            doClose();
            return;
        }

        fileRes_.reset();

        if (close) {
            return doClose();
        }

        doRead();
    }

    void Session::doClose() {
        beast::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }

} // namespace learnChemistry::server