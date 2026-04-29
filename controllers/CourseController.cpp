#include "controllers/CourseController.h"

#include "services/CourseService.h"
#include "utils/HttpResponse.h"

#include <nlohmann/json.hpp>
#include <string>
#include <cctype>

namespace learnChemistry::controllers {

    CourseController::CourseController(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    static std::string stripQuery(std::string s) {
        auto q = s.find('?');
        if (q != std::string::npos) s.resize(q);
        if (s.size() > 1 && s.back() == '/') s.pop_back();
        return s;
    }

    static std::string getQueryValue(const std::string& url, const std::string& key) {
        auto q = url.find('?');
        if (q == std::string::npos) return {};
        std::string query = url.substr(q + 1);

        // very small query parser for page/limit/search
        size_t start = 0;
        while (start < query.size()) {
            auto amp = query.find('&', start);
            std::string pair = query.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
            auto eq = pair.find('=');
            std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
            std::string v = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
            if (k == key) return v;
            if (amp == std::string::npos) break;
            start = amp + 1;
        }
        return {};
    }

    static int toIntOr(const std::string& s, int def) {
        if (s.empty()) return def;
        try { return std::stoi(s); }
        catch (...) { return def; }
    }

    static bool isDigits(const std::string& s) {
        if (s.empty()) return false;
        for (char ch : s) if (!std::isdigit(static_cast<unsigned char>(ch))) return false;
        return true;
    }

    http::response<http::string_body>
        CourseController::list(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext&) const
    {
        using learnChemistry::utils::HttpResponse;

        const std::string target = std::string(req.target());
        const int page = toIntOr(getQueryValue(target, "page"), 1);
        const int limit = toIntOr(getQueryValue(target, "limit"), 12);
        const std::string search = getQueryValue(target, "search");

        learnChemistry::services::CourseService svc(pool_);
        auto result = svc.listActiveCourses(page, limit, search);

        nlohmann::json items = nlohmann::json::array();
        for (const auto& c : result.items) {
            items.push_back({
                {"id", c.id},
                {"slug", c.slug},
                {"title", c.title},
                {"description", c.description},
                {"pricePaise", c.pricePaise},
                {"currency", c.currency},
                {"thumbnailUrl", c.thumbnailUrl},
                {"isActive", c.isActive}
                });
        }

        nlohmann::json body = {
            {"items", items},
            {"page", page},
            {"limit", limit},
            {"total", result.total}
        };

        return HttpResponse::json(http::status::ok, body);
    }

    http::response<http::string_body>
        CourseController::detail(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext&) const
    {
        using learnChemistry::utils::HttpResponse;

        // expected path: /v1/courses/<id>
        std::string path = stripQuery(std::string(req.target()));
        const std::string prefix = "/v1/courses/";
        if (path.rfind(prefix, 0) != 0) {
            return HttpResponse::json(http::status::not_found, { {"error","not found"} });
        }

        std::string idStr = path.substr(prefix.size());
        if (!isDigits(idStr)) {
            return HttpResponse::json(http::status::bad_request, { {"error","invalid courseId"} });
        }

        long long courseId = 0;
        try { courseId = std::stoll(idStr); }
        catch (...) { courseId = 0; }
        if (courseId <= 0) {
            return HttpResponse::json(http::status::bad_request, { {"error","invalid courseId"} });
        }

        learnChemistry::services::CourseService svc(pool_);
        auto cOpt = svc.getActiveCourseById(courseId);

        if (!cOpt) {
            return HttpResponse::json(http::status::not_found, { {"error","course not found"} });
        }

        const auto& c = *cOpt;
        nlohmann::json body = {
            {"id", c.id},
            {"slug", c.slug},
            {"title", c.title},
            {"description", c.description},
            {"pricePaise", c.pricePaise},
            {"currency", c.currency},
            {"thumbnailUrl", c.thumbnailUrl},
            {"isActive", c.isActive}
        };

        return HttpResponse::json(http::status::ok, body);
    }

} // namespace learnChemistry::controllers
