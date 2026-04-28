#include "utils/HttpResponse.h"

namespace learnChemistry::utils {

    http::response<http::string_body>
        HttpResponse::json(http::status status, const nlohmann::json& body)
    {
        http::response<http::string_body> res{ status, 11 };
        res.set(http::field::content_type, "application/json; charset=utf-8");
        res.body() = body.dump(
            -1, ' ', false,
            nlohmann::json::error_handler_t::replace
        );

        res.prepare_payload();
        return res;
    }

} // namespace learnChemistry::utils

