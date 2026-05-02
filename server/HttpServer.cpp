#include "server/HttpServer.h"

#include "security/JwtService.h"
#include "utils/HttpResponse.h"
#include "server/Session.h"

#include "controllers/AuthController.h"
#include "controllers/CourseController.h"
#include "controllers/OrderController.h"
#include "controllers/MeController.h"
#include "controllers/AdminController.h"
#include "controllers/DownloadController.h"

#include <boost/asio/ip/address.hpp>
#include <memory>
#include <utility>
#include <iostream>

namespace learnChemistry::server {

    HttpServer::HttpServer(const learnChemistry::config::AppConfig& cfg)
        : cfg_(cfg),
        ioc_(1),
        acceptor_(ioc_, { boost::asio::ip::make_address(cfg_.host), cfg_.port })
    {
        pool_.init(cfg_);

        const std::string secret = cfg_.jwt_secret;

        // =========================
        // Auth routes
        // =========================
        auto makeAuthHandler = [this, secret](auto handlerFn) {
            return [this, secret, handlerFn](const http::request<http::string_body>& req,
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

        // =========================
        // Courses (public)
        // =========================
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

        // =========================
        // Orders (JWT)
        // =========================
        router_.addRoute(http::verb::post, "/v1/orders/free",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::OrderController controller(this->dbPool());
                return controller.freePurchase(req, ctx);
            });

        // =========================
        // Me (JWT)
        // =========================
        router_.addRoute(http::verb::get, "/v1/me/courses",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::MeController controller(this->dbPool());
                return controller.myCourses(req, ctx);
            });

        // =========================
        // Admin (JWT + ADMIN enforced inside AdminController)
        // =========================
        router_.addRoute(http::verb::post, "/v1/admin/courses",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::AdminController controller(this->dbPool());
                return controller.createCourse(req, ctx);
            });

        router_.addRoute(http::verb::post, "/v1/admin/course-pdf/:id",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::AdminController controller(this->dbPool());
                return controller.uploadCoursePdf(req, ctx);
            });

        router_.addRoute(http::verb::get, "/v1/admin/orders",
            [this](const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx)
            {
                learnChemistry::controllers::AdminController controller(this->dbPool());
                return controller.listOrders(req, ctx);
            });

        // ✅ IMPORTANT:
        // Do NOT register /v1/download/:id in router_. Download uses http::file_body.
        // It is served via HttpServer::handleDownload() and Session intercepts /v1/download.
    }

    void HttpServer::run() {
        doAccept();
        try {
            ioc_.run();
        }
        catch (const std::exception& ex) {
            std::cerr << "FATAL: exception escaped io_context::run(): " << ex.what() << "\n";
        }
    }

    void HttpServer::doAccept() {
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

    std::string HttpServer::getOrigin(const http::request<http::string_body>& req) {
        auto it = req.base().find(http::field::origin);
        if (it == req.base().end()) return {};
        return std::string(it->value());
    }

    void HttpServer::addCorsHeaders(http::response<http::string_body>& res, const std::string& origin)
    {
        const bool allowed =
            (origin == "http://localhost:5500") ||
            (origin == "http://127.0.0.1:5500");

        if (allowed) {
            res.set(http::field::access_control_allow_origin, origin);
            res.set(http::field::vary, "Origin");
        }

        res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type, Authorization, X-Filename");

        // ✅ allow JS to read headers like Content-Disposition
        res.set(http::field::access_control_expose_headers, "Content-Disposition, Content-Type");

        res.set(http::field::access_control_max_age, "600");
    }

    void HttpServer::addCorsHeaders(http::response<http::file_body>& res, const std::string& origin)
    {
        const bool allowed =
            (origin == "http://localhost:5500") ||
            (origin == "http://127.0.0.1:5500");

        if (allowed) {
            res.set(http::field::access_control_allow_origin, origin);
            res.set(http::field::vary, "Origin");
        }

        res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type, Authorization, X-Filename");

        // ✅ allow JS to read headers like Content-Disposition
        res.set(http::field::access_control_expose_headers, "Content-Disposition, Content-Type");

        res.set(http::field::access_control_max_age, "600");
    }

    http::response<http::string_body>
        HttpServer::handleRequest(const http::request<http::string_body>& req)
    {
        const std::string origin = getOrigin(req);

        // Preflight
        if (req.method() == http::verb::options) {
            http::response<http::string_body> res{ http::status::no_content, req.version() };
            addCorsHeaders(res, origin);
            res.keep_alive(req.keep_alive());
            return res;
        }

        learnChemistry::context::RequestContext ctx;

        // Normalize target
        std::string target = std::string(req.target());
        if (auto q = target.find('?'); q != std::string::npos) target.resize(q);
        if (target.size() > 1 && target.back() == '/') target.pop_back();

        // JWT protection
        const bool requiresAuth =
            (target.rfind("/v1/me", 0) == 0) ||
            (target.rfind("/v1/orders", 0) == 0) ||
            (target.rfind("/v1/admin", 0) == 0);

        if (requiresAuth) {
            auto it = req.find(http::field::authorization);
            if (it == req.end()) {
                auto res = learnChemistry::utils::HttpResponse::json(
                    http::status::unauthorized, { {"error","Missing Authorization header"} });
                addCorsHeaders(res, origin);
                res.version(req.version());
                res.keep_alive(req.keep_alive());
                return res;
            }

            const std::string auth = std::string(it->value());
            const std::string prefix = "Bearer ";
            if (auth.rfind(prefix, 0) != 0) {
                auto res = learnChemistry::utils::HttpResponse::json(
                    http::status::unauthorized, { {"error","Invalid Authorization format"} });
                addCorsHeaders(res, origin);
                res.version(req.version());
                res.keep_alive(req.keep_alive());
                return res;
            }

            const std::string token = auth.substr(prefix.size());
            auto userOpt = learnChemistry::security::JwtService::verify(token, cfg_.jwt_secret);

            if (!userOpt) {
                auto res = learnChemistry::utils::HttpResponse::json(
                    http::status::unauthorized, { {"error","Invalid or expired token"} });
                addCorsHeaders(res, origin);
                res.version(req.version());
                res.keep_alive(req.keep_alive());
                return res;
            }

            ctx.user = *userOpt;
        }

        // Route match (string only)
        auto route = router_.match(req.method(), target);

        http::response<http::string_body> res;
        if (!route) {
            res = learnChemistry::utils::HttpResponse::json(
                http::status::not_found, { {"error","not found"}, {"path", target} });
        }
        else {
            res = route->handler(req, ctx);
        }

        addCorsHeaders(res, origin);
        res.version(req.version());
        res.keep_alive(req.keep_alive());
        return res;
    }

    http::response<http::file_body>
        HttpServer::handleDownload(const http::request<http::string_body>& req)
    {
        const std::string origin = getOrigin(req);

        learnChemistry::context::RequestContext ctx;

        // JWT required
        auto it = req.find(http::field::authorization);
        if (it == req.end()) {
            http::response<http::file_body> res{ http::status::unauthorized, req.version() };
            addCorsHeaders(res, origin);
            res.keep_alive(req.keep_alive());
            return res;
        }

        const std::string auth = std::string(it->value());
        const std::string prefix = "Bearer ";
        if (auth.rfind(prefix, 0) != 0) {
            http::response<http::file_body> res{ http::status::unauthorized, req.version() };
            addCorsHeaders(res, origin);
            res.keep_alive(req.keep_alive());
            return res;
        }

        const std::string token = auth.substr(prefix.size());
        auto userOpt = learnChemistry::security::JwtService::verify(token, cfg_.jwt_secret);
        if (!userOpt) {
            http::response<http::file_body> res{ http::status::unauthorized, req.version() };
            addCorsHeaders(res, origin);
            res.keep_alive(req.keep_alive());
            return res;
        }

        ctx.user = *userOpt;

        learnChemistry::controllers::DownloadController controller(this->dbPool());
        auto res = controller.downloadPdf(req, ctx);

        addCorsHeaders(res, origin);
        res.version(req.version());
        res.keep_alive(req.keep_alive());
        return res;
    }

    learnChemistry::db::MySqlPool& HttpServer::dbPool() {
        return pool_;
    }

} // namespace learnChemistry::server