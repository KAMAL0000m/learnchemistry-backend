#include "server/HttpServer.h"

#include "server/Session.h"
#include "controllers/AuthController.h"
#include "utils/HttpResponse.h"
#include "controllers/CourseController.h"
#include <boost/asio/ip/address.hpp>
#include <memory>
#include <utility>
#include <iostream>


namespace learnChemistry::server {

    HttpServer::HttpServer(const learnChemistry::config::AppConfig& cfg)
        : cfg_(cfg),
        ioc_(1), // keep 1 for debugging; you can change to cfg_.threads later
        acceptor_(ioc_, { boost::asio::ip::make_address(cfg_.host), cfg_.port })
    {
        pool_.init(cfg_);
        // ✅ Register route (POST /v1/auth/login)
        // Avoid capturing 'this' unless needed; capture secret by value (cleaner & safer)
        const std::string secret = cfg_.jwt_secret;

        auto makeAuthHandler = [this,secret](auto handlerFn) {
            return [this,secret, handlerFn](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
                {
                    learnChemistry::controllers::AuthController controller(secret, this->dbPool());
                    return (controller.*handlerFn)(req, ctx);
                };
            };
        router_.addRoute(http::verb::post, "/v1/auth/login",
            makeAuthHandler(&learnChemistry::controllers::AuthController::login));
        router_.addRoute(http::verb::post, "/v1/auth/signup",
            makeAuthHandler(&learnChemistry::controllers::AuthController::signup));


        router_.addRoute(http::verb::get, "/v1/courses",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::CourseController controller(this->dbPool());
                return controller.list(req, ctx);
            });

        router_.addRoute(http::verb::get, "/v1/courses/:id",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::CourseController controller(this->dbPool());
                return controller.detail(req, ctx);
            });
    }

    void HttpServer::run() {
        doAccept();
        //ioc_.run();
        try {
            ioc_.run();
        }
        catch (const std::exception& ex) {
            // If a handler throws, it will escape here
            std::cerr << "FATAL: exception escaped io_context::run(): " << ex.what() << "\n";
        }


    }

    void HttpServer::doAccept() {
        // ✅ strand is safer if you later increase threads
        acceptor_.async_accept(
            boost::asio::make_strand(ioc_),
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), *this)->start();
                }
                doAccept();
            }
        );
    }

    // ✅ No .to_string() anywhere. Convert beast::string_view using std::string(...)
    std::string HttpServer::getOrigin(const http::request<http::string_body>& req) {
        const auto& headers = req.base();
        auto it = headers.find(http::field::origin);
        if (it == headers.end()) return {};
        return std::string(it->value());  // beast::string_view -> std::string
    }

    void HttpServer::addCorsHeaders(http::response<http::string_body>& res, const std::string& origin) {
        // ✅ Allow only local UI origins (add your production UI later)
        const bool allowed =
            (origin == "http://localhost:5500") ||
            (origin == "http://127.0.0.1:5500");
        // || (origin == "https://app.learnchemistry.com");

        if (allowed) {
            res.set(http::field::access_control_allow_origin, origin);
            res.set(http::field::vary, "Origin");
        }

        res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
        res.set(http::field::access_control_max_age, "600");
    }

    http::response<http::string_body>
        HttpServer::handleRequest(const http::request<http::string_body>& req)
    {
        const std::string origin = getOrigin(req);

        // ✅ 1) CORS preflight: browser sends OPTIONS first
        if (req.method() == http::verb::options) {
            http::response<http::string_body> res{ http::status::no_content, req.version() };
            addCorsHeaders(res, origin);
            res.keep_alive(req.keep_alive());
            return res;
        }

        learnChemistry::context::RequestContext ctx;

        // ✅ 2) Normalize target (strip query + trailing slash)
        std::string target = std::string(req.target());

        if (auto q = target.find('?'); q != std::string::npos)
            target.resize(q);

        if (target.size() > 1 && target.back() == '/')
            target.pop_back();

        // ✅ 3) Route match
        auto route = router_.match(req.method(), target);

        http::response<http::string_body> res;

        if (!route) {
            res = learnChemistry::utils::HttpResponse::json(
                http::status::not_found,
                { {"error", "not found"}, {"path", target} }
            );
        }
        else {
            res = route->handler(req, ctx);
        }

        // ✅ 4) Add CORS headers to *all* responses (so browser can read response)
        addCorsHeaders(res, origin);

        // ✅ 5) Preserve request HTTP version & keep-alive
        res.version(req.version());
        res.keep_alive(req.keep_alive());

        return res;
    }

    learnChemistry::db::MySqlPool& HttpServer::dbPool() {
        return pool_;
    }

} // namespace learnChemistry::server