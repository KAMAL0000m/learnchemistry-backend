#pragma once

#include <boost/beast/http.hpp>
#include "db/MySqlPool.h"
#include "context/RequestContext.h"

namespace learnChemistry::controllers {

    namespace http = boost::beast::http;

    class AdminController {
    public:
        explicit AdminController(learnChemistry::db::MySqlPool& pool);

        http::response<http::string_body>
            createCourse(const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx) const;

        http::response<http::string_body>
            uploadCoursePdf(const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx) const;

        http::response<http::string_body>
            uploadCourseThumb(const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx) const;

        http::response<http::string_body>
            listOrders(const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext& ctx) const;

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::controllers