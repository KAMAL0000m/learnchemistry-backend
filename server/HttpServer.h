#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>

#include "config/AppConfig.h"
#include "routing/Router.h"
#include "context/RequestContext.h"
#include "db/MySqlPool.h"

namespace learnChemistry::server {

    // ✅ Namespace alias MUST be at namespace scope (OK here)
    namespace http = boost::beast::http;

    class HttpServer {
    public:
        using tcp = boost::asio::ip::tcp;

        explicit HttpServer(const learnChemistry::config::AppConfig& cfg);
        void run();

        http::response<http::string_body>
            handleRequest(const http::request<http::string_body>& req);

        learnChemistry::db::MySqlPool& dbPool();

    private:
        void doAccept();

        // CORS helpers
        static std::string getOrigin(const http::request<http::string_body>& req);
        static void addCorsHeaders(http::response<http::string_body>& res, const std::string& origin);

    private:
        learnChemistry::config::AppConfig cfg_;

        boost::asio::io_context ioc_;
        tcp::acceptor acceptor_;
        learnChemistry::db::MySqlPool pool_;
        learnChemistry::routing::Router router_;
    };

} // namespace learnChemistry::server