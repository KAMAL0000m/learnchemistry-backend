
#pragma once
#include <boost/beast/http.hpp>
#include <functional>
#include <string>
#include "context/RequestContext.h"

namespace learnChemistry::routing {

    namespace http = boost::beast::http;

    using HandlerFn = std::function<
        http::response<http::string_body>(
            const http::request<http::string_body>&,
            learnChemistry::context::RequestContext&
        )
    >;

    struct Route {
        http::verb method;
        std::string path;
        HandlerFn handler;
    };

}
