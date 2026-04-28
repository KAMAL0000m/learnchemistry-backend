#include "controllers/AuthController.h"
#include "services/AuthService.h"
#include "utils/HttpResponse.h"

#include <nlohmann/json.hpp>
#include <string>

namespace learnChemistry::controllers 
{


    AuthController::AuthController(std::string secret, learnChemistry::db::MySqlPool& pool)
        : jwtSecret_(std::move(secret)), pool_(pool) {
    }

    http::response<http::string_body>
        AuthController::login(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext&) const
    {
        using learnChemistry::utils::HttpResponse;

        // Optional: reject non-JSON content types (helps debugging)
        auto ctIt = req.find(http::field::content_type);
        std::string ct = (ctIt != req.end()) ? std::string(ctIt->value()) : "";
        if (ct.find("application/json") == std::string::npos) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","Content-Type must be application/json"}, {"contentType", ct} });
        }

        // ✅ Parse safely: allow_exceptions = false
        nlohmann::json body = nlohmann::json::parse(req.body(), nullptr, false);

        if (body.is_discarded() || !body.is_object()) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","Invalid JSON or invalid UTF-8 in body"} });
        }

        if (!body.contains("email") || !body["email"].is_string() ||
            !body.contains("password") || !body["password"].is_string()) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","email and password must be strings"} });
        }

        const std::string email = body["email"].get<std::string>();
        const std::string password = body["password"].get<std::string>();

        if (email.empty() || password.empty()) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","missing credentials"} });
        }

        try {
            auto svc = learnChemistry::services::AuthService(pool_);
            auto result = svc.login(email, password, jwtSecret_);
            return HttpResponse::json(http::status::ok,
                { {"token", result.token}, {"userId", result.userId} });
        }
        catch (const std::exception& ex) {
            // invalid credentials etc.
            return HttpResponse::json(http::status::unauthorized,
                { {"error", ex.what()} });
        }
    }

    http::response<http::string_body>
        AuthController::signup(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext&) const
    {
        using learnChemistry::utils::HttpResponse;

        // 1. Content-Type check
        auto ctIt = req.find(http::field::content_type);
        std::string ct = (ctIt != req.end()) ? std::string(ctIt->value()) : "";

        if (ct.find("application/json") == std::string::npos) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","Content-Type must be application/json"} });
        }

        // 2. Parse JSON
        nlohmann::json body = nlohmann::json::parse(req.body(), nullptr, false);

        if (body.is_discarded() || !body.is_object()) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","Invalid JSON"} });
        }

        // 3. Validate fields
        if (!body.contains("email") || !body["email"].is_string() ||
            !body.contains("password") || !body["password"].is_string()) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","email and password must be strings"} });
        }

        const std::string email = body["email"].get<std::string>();
        const std::string password = body["password"].get<std::string>();

        if (email.empty() || password.empty()) {
            return HttpResponse::json(http::status::bad_request,
                { {"error","missing credentials"} });
        }

        try {
            auto svc = learnChemistry::services::AuthService(pool_);
            auto result = svc.signup(email, password, jwtSecret_);
            return HttpResponse::json(http::status::created,
                { {"message","User registered successfully"},
                  {"userId", result.userId},
                  {"token", result.token}
                });
        }
        catch (const std::exception& ex) {
            return HttpResponse::json(http::status::bad_request,
                { {"error", ex.what()} });
        }
    }

} // namespace learnChemistry::controllers
