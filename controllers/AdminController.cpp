#include "controllers/AdminController.h"

#include "services/AdminService.h"
#include "utils/HttpResponse.h"
#include <nlohmann/json.hpp>

#include <string>
#include <cctype>

namespace learnChemistry::controllers {

    AdminController::AdminController(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    static bool isAdmin(const learnChemistry::context::RequestContext& ctx) {
        return ctx.user.has_value() && ctx.user->role == "ADMIN";
    }

    static std::string stripQuery(std::string s) {
        auto q = s.find('?');
        if (q != std::string::npos) s.resize(q);
        if (s.size() > 1 && s.back() == '/') s.pop_back();
        return s;
    }

    static bool isDigits(const std::string& s) {
        if (s.empty()) return false;
        for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    }

    http::response<http::string_body>
        AdminController::createCourse(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const
    {
        using learnChemistry::utils::HttpResponse;

        if (!isAdmin(ctx)) return HttpResponse::json(http::status::forbidden, { {"error","Admin only"} });

        auto body = nlohmann::json::parse(req.body(), nullptr, false);
        if (body.is_discarded() || !body.is_object()) {
            return HttpResponse::json(http::status::bad_request, { {"error","Invalid JSON"} });
        }

        const std::string title = body.value("title", "");
        const std::string exam = body.value("exam", "");
        const int priceInr = body.value("priceInr", 0);
        const std::string desc = body.value("description", "");

        if (title.empty()) return HttpResponse::json(http::status::bad_request, { {"error","title required"} });

        try {
            learnChemistry::services::AdminService svc(pool_);
            long long courseId = svc.createCourse(title, exam, priceInr, desc);
            return HttpResponse::json(http::status::created, { {"courseId", courseId} });
        }
        catch (const std::exception& ex) {
            return HttpResponse::json(http::status::bad_request, { {"error", ex.what()} });
        }
    }

    http::response<http::string_body>
        AdminController::uploadCoursePdf(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const
    {
        using learnChemistry::utils::HttpResponse;

        if (!isAdmin(ctx)) return HttpResponse::json(http::status::forbidden, { {"error","Admin only"} });

        // /v1/admin/course-pdf/<id>
        std::string path = stripQuery(std::string(req.target()));
        const std::string prefix = "/v1/admin/course-pdf/";
        if (path.rfind(prefix, 0) != 0) return HttpResponse::json(http::status::not_found, { {"error","not found"} });

        std::string idStr = path.substr(prefix.size());
        if (!isDigits(idStr)) return HttpResponse::json(http::status::bad_request, { {"error","invalid courseId"} });

        long long courseId = 0;
        try { courseId = std::stoll(idStr); }
        catch (...) { courseId = 0; }
        if (courseId <= 0) return HttpResponse::json(http::status::bad_request, { {"error","invalid courseId"} });

        // must be PDF
        auto ctIt = req.find(http::field::content_type);
        std::string ct = (ctIt != req.end()) ? std::string(ctIt->value()) : "";
        if (ct.find("application/pdf") == std::string::npos) {
            return HttpResponse::json(http::status::bad_request, { {"error","Content-Type must be application/pdf"} });
        }

        std::string filename;
        auto fnIt = req.find("X-Filename");
        if (fnIt != req.end()) filename = std::string(fnIt->value());

        try {
            learnChemistry::services::AdminService svc(pool_);
            auto out = svc.storePdf(courseId, req.body(), filename, ct);
            return HttpResponse::json(http::status::created, {
                {"courseId", out.courseId},
                {"pdfPath", out.pdfPath}
                });
        }
        catch (const std::exception& ex) {
            return HttpResponse::json(http::status::bad_request, { {"error", ex.what()} });
        }
    }

    http::response<http::string_body>
        AdminController::uploadCourseThumb(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const
    {
        using learnChemistry::utils::HttpResponse;

        if (!isAdmin(ctx)) return HttpResponse::json(http::status::forbidden, { {"error","Admin only"} });

        // /v1/admin/course-thumb/<id>
        std::string path = stripQuery(std::string(req.target()));
        const std::string prefix = "/v1/admin/course-thumb/";
        if (path.rfind(prefix, 0) != 0) return HttpResponse::json(http::status::not_found, { {"error","not found"} });

        std::string idStr = path.substr(prefix.size());
        if (!isDigits(idStr)) return HttpResponse::json(http::status::bad_request, { {"error","invalid courseId"} });

        long long courseId = 0;
        try { courseId = std::stoll(idStr); }
        catch (...) { courseId = 0; }
        if (courseId <= 0) return HttpResponse::json(http::status::bad_request, { {"error","invalid courseId"} });

        // must be image/*
        auto ctIt = req.find(http::field::content_type);
        std::string ct = (ctIt != req.end()) ? std::string(ctIt->value()) : "";
        if (ct.rfind("image/", 0) != 0) {
            return HttpResponse::json(http::status::bad_request, { {"error","Content-Type must be image/*"} });
        }

        std::string filename;
        auto fnIt = req.find("X-Filename");
        if (fnIt != req.end()) filename = std::string(fnIt->value());

        try {
            learnChemistry::services::AdminService svc(pool_);
            auto out = svc.storeThumbnail(courseId, req.body(), filename, ct);
            return HttpResponse::json(http::status::created, {
                {"courseId", out.courseId},
                {"thumbUrl", out.thumbUrl},
                {"thumbPath", out.thumbPath}
                });
        }
        catch (const std::exception& ex) {
            return HttpResponse::json(http::status::bad_request, { {"error", ex.what()} });
        }
    }

    http::response<http::string_body>
        AdminController::listOrders(const http::request<http::string_body>&,
            learnChemistry::context::RequestContext& ctx) const
    {
        using learnChemistry::utils::HttpResponse;

        if (!isAdmin(ctx)) return HttpResponse::json(http::status::forbidden, { {"error","Admin only"} });

        learnChemistry::services::AdminService svc(pool_);
        return HttpResponse::json(http::status::ok, svc.listOrders());
    }

} // namespace learnChemistry::controllers