#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include "config/AppConfig.h"
#include "routing/Router.h"
#include "db/MySqlPool.h"

namespace learnChemistry::server {

    namespace http = boost::beast::http;

    class HttpServer {
    public:
        using tcp = boost::asio::ip::tcp;

        explicit HttpServer(const learnChemistry::config::AppConfig& cfg);

        void run();

        http::response<http::string_body>
            handleRequest(const http::request<http::string_body>& req);

        // ✅ file download
        http::response<http::file_body>
            handleDownload(const http::request<http::string_body>& req);

        learnChemistry::db::MySqlPool& dbPool();

    private:
        void doAccept();

        static std::string getOrigin(const http::request<http::string_body>& req);

        static void addCorsHeaders(http::response<http::string_body>& res, const std::string& origin);
        static void addCorsHeaders(http::response<http::file_body>& res, const std::string& origin);

    private:
        learnChemistry::config::AppConfig cfg_;
        boost::asio::io_context ioc_;
        tcp::acceptor acceptor_;

        learnChemistry::routing::Router router_;
        learnChemistry::db::MySqlPool pool_;
    };

} // namespace learnChemistry::server