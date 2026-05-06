#pragma once

#include <boost/beast/http.hpp>
#include "db/MySqlPool.h"

namespace learnChemistry::controllers {

    namespace http = boost::beast::http;

    class ThumbController {
    public:
        explicit ThumbController(learnChemistry::db::MySqlPool& pool);

        http::response<http::file_body>
            getThumb(const http::request<http::string_body>& req) const;

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::controllers