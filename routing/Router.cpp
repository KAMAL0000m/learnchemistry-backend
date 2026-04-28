
#include "routing/Router.h"

namespace learnChemistry::routing {

    void Router::addRoute(http::verb method,
        const std::string& path,
        HandlerFn handler) {
        routes_.push_back({ method, path, std::move(handler) });
    }

    std::optional<Route>
        Router::match(http::verb method,
            const std::string& path) const {
        for (const auto& r : routes_) 
        {
            if (r.method == method && r.path == path)
                return r;
        }
        return std::nullopt;
    }

}
