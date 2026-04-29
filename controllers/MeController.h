#pragma once
#include <boost/beast/http.hpp>
#include "db/MySqlPool.h"
#include "context/RequestContext.h"

namespace learnChemistry::controllers {
    namespace http = boost::beast::http;

    class MeController {
    public:
        explicit MeController(learnChemistry::db::MySqlPool& pool);

        http::response<http::string_body> myCourses(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const;
    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::controllers