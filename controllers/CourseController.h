#pragma once
#include <boost/beast/http.hpp>
#include <string>

#include "db/MySqlPool.h"
#include "context/RequestContext.h"

namespace learnChemistry::controllers {

    namespace http = boost::beast::http;

    class CourseController {
    public:
        explicit CourseController(learnChemistry::db::MySqlPool& pool);

        http::response<http::string_body> list(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const;

        http::response<http::string_body> detail(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const;

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::controllers