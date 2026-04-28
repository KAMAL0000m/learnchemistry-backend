#pragma once
#include <boost/beast/http.hpp>
#include <string>
#include "context/RequestContext.h"
#include "db/MySqlPool.h"

namespace learnChemistry::controllers {

    namespace http = boost::beast::http;

    class AuthController {
    public:
        AuthController(std::string jwtSecret, learnChemistry::db::MySqlPool& pool);

        http::response<http::string_body>
            login(const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext&) const;

        http::response<http::string_body>
            signup(const http::request<http::string_body>& req,
                learnChemistry::context::RequestContext&) const;

    private:
        std::string jwtSecret_;
        learnChemistry::db::MySqlPool& pool_;
    };

}
