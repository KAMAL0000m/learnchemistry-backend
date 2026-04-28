
#pragma once
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace learnChemistry::utils {

    namespace http = boost::beast::http;

    class HttpResponse {
    public:
        static http::response<http::string_body>
            json(http::status status, const nlohmann::json& body);
    };

}
