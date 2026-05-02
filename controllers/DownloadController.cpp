#include "controllers/DownloadController.h"

#include "services/DownloadService.h"

#include <boost/beast/core/file.hpp>
#include <string>
#include <cctype>

namespace learnChemistry::controllers {

    DownloadController::DownloadController(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
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

    http::response<http::file_body>
        DownloadController::downloadPdf(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const
    {
        namespace beast = boost::beast;

        // must be logged in
        if (!ctx.user.has_value()) {
            http::response<http::file_body> res{ http::status::unauthorized, req.version() };
            res.keep_alive(req.keep_alive());
            return res;
        }

        // path: /v1/download/<courseId>
        std::string path = stripQuery(std::string(req.target()));
        const std::string prefix = "/v1/download/";
        if (path.rfind(prefix, 0) != 0) {
            http::response<http::file_body> res{ http::status::not_found, req.version() };
            res.keep_alive(req.keep_alive());
            return res;
        }

        std::string idStr = path.substr(prefix.size());
        if (!isDigits(idStr)) {
            http::response<http::file_body> res{ http::status::bad_request, req.version() };
            res.keep_alive(req.keep_alive());
            return res;
        }

        long long courseId = 0;
        try { courseId = std::stoll(idStr); }
        catch (...) { courseId = 0; }
        if (courseId <= 0) {
            http::response<http::file_body> res{ http::status::bad_request, req.version() };
            res.keep_alive(req.keep_alive());
            return res;
        }

        // service checks enrollment + finds asset path
        learnChemistry::services::DownloadService svc(pool_);
        auto infoOpt = svc.getPdfForUser(ctx.user->id, courseId);

        if (!infoOpt) {
            // either not enrolled OR no pdf
            http::response<http::file_body> res{ http::status::forbidden, req.version() };
            res.keep_alive(req.keep_alive());
            return res;
        }

        const auto& info = *infoOpt;

        // open file
        beast::error_code ec;
        http::file_body::value_type body;
        body.open(info.filePath.c_str(), beast::file_mode::scan, ec);
        if (ec) {
            http::response<http::file_body> res{ http::status::not_found, req.version() };
            res.keep_alive(req.keep_alive());
            return res;
        }

        auto const size = body.size();

        http::response<http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())
        };

        res.set(http::field::content_type, info.mimeType);
        res.set(http::field::content_disposition, "attachment; filename=\"" + info.filename + "\"");
        res.content_length(size);
        res.keep_alive(req.keep_alive());

        return res;
    }

} // namespace learnChemistry::controllers
