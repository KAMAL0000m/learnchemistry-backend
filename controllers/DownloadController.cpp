#include "controllers/DownloadController.h"
#include "services/DownloadService.h"

#include <boost/beast/core/file.hpp>
#include <cctype>
#include <string>

namespace learnChemistry::controllers {

    static std::string stripQuery(std::string s) {
        auto q = s.find('?');
        if (q != std::string::npos) s.resize(q);
        if (s.size() > 1 && s.back() == '/') s.pop_back();
        return s;
    }

    static bool isDigits(const std::string& s) {
        if (s.empty()) return false;
        for (char c : s) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        }
        return true;
    }

    // ✅ Safe empty file_body response to prevent async_write crashes
    static http::response<http::file_body>
        makeEmptyFileRes(http::status st, const http::request<http::string_body>& req) {
        http::response<http::file_body> res{ st, req.version() };
        res.set(http::field::content_type, "text/plain");
        res.content_length(0);                 // ✅ critical for file_body error responses
        res.keep_alive(req.keep_alive());
        return res;
    }

    DownloadController::DownloadController(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    http::response<http::file_body>
        DownloadController::downloadPdf(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const
    {
        namespace beast = boost::beast;

        // must be logged in
        if (!ctx.user.has_value()) {
            return makeEmptyFileRes(http::status::unauthorized, req);
        }

        // path: /v1/download/<courseId>
        std::string path = stripQuery(std::string(req.target()));
        const std::string prefix = "/v1/download/";
        if (path.rfind(prefix, 0) != 0) {
            return makeEmptyFileRes(http::status::not_found, req);
        }

        std::string idStr = path.substr(prefix.size());
        if (!isDigits(idStr)) {
            return makeEmptyFileRes(http::status::bad_request, req);
        }

        long long courseId = 0;
        try { courseId = std::stoll(idStr); }
        catch (...) { courseId = 0; }
        if (courseId <= 0) {
            return makeEmptyFileRes(http::status::bad_request, req);
        }

        // service checks enrollment + finds pdf path
        learnChemistry::services::DownloadService svc(pool_);
        auto infoOpt = svc.getPdfForUser(ctx.user->id, courseId);

        if (!infoOpt) {
            // either not enrolled OR no pdf
            return makeEmptyFileRes(http::status::forbidden, req);
        }

        const auto& info = *infoOpt;

        // open file for streaming (READ mode)
        beast::error_code ec;
        http::file_body::value_type body;
        body.open(info.filePath.c_str(), beast::file_mode::read, ec);   // ✅ read, not scan
        if (ec) {
            return makeEmptyFileRes(http::status::not_found, req);
        }

        auto const size = body.size();

        http::response<http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())
        };

        const std::string mime = info.mimeType.empty() ? "application/pdf" : info.mimeType;
        res.set(http::field::content_type, mime);
        res.set(http::field::content_disposition, "attachment; filename=\"" + info.filename + "\"");
        res.content_length(size);
        res.keep_alive(req.keep_alive());

        return res;
    }

} // namespace learnChemistry::controllers