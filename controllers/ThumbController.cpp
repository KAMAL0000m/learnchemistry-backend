#include "controllers/ThumbController.h"
#include <boost/beast/core/file.hpp>
#include <cctype>
#include <string>

namespace learnChemistry::controllers {

    ThumbController::ThumbController(learnChemistry::db::MySqlPool& pool)
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

    static std::string guessMimeFromPath(const std::string& p, const std::string& storedMime) {
        if (!storedMime.empty()) return storedMime;
        auto dot = p.find_last_of('.');
        if (dot == std::string::npos) return "image/jpeg";
        std::string ext = p.substr(dot + 1);
        for (auto& ch : ext) ch = (char)std::tolower((unsigned char)ch);
        if (ext == "png") return "image/png";
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "webp") return "image/webp";
        return "image/jpeg";
    }

    http::response<http::file_body>
        ThumbController::getThumb(const http::request<http::string_body>& req) const
    {
        namespace beast = boost::beast;
        namespace http = boost::beast::http;

        std::string path = stripQuery(std::string(req.target()));
        const std::string prefix = "/v1/thumb/";
        if (path.rfind(prefix, 0) != 0) {
            return http::response<http::file_body>{ http::status::not_found, req.version() };
        }

        std::string idStr = path.substr(prefix.size());
        if (!isDigits(idStr)) {
            return http::response<http::file_body>{ http::status::bad_request, req.version() };
        }

        long long courseId = 0;
        try { courseId = std::stoll(idStr); }
        catch (...) { courseId = 0; }
        if (courseId <= 0) {
            return http::response<http::file_body>{ http::status::bad_request, req.version() };
        }

        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        auto row = sess.sql("SELECT thumbnail_path, thumbnail_mime FROM courses WHERE id=? LIMIT 1")
            .bind(courseId).execute().fetchOne();

        if (!row || row[0].isNull()) {
            return http::response<http::file_body>{ http::status::not_found, req.version() };
        }

        const std::string thumbPath = row[0].get<std::string>();
        const std::string thumbMime = row[1].isNull() ? "" : row[1].get<std::string>();

        beast::error_code ec;
        http::file_body::value_type body;
        body.open(thumbPath.c_str(), beast::file_mode::read, ec);
        if (ec) {
            return http::response<http::file_body>{ http::status::not_found, req.version() };
        }

        auto const size = body.size();

        http::response<http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())
        };

        res.set(http::field::content_type, guessMimeFromPath(thumbPath, thumbMime));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

} // namespace learnChemistry::controllers