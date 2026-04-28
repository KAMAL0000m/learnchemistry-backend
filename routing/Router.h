
#pragma once
#include <vector>
#include <optional>
#include "routing/Route.h"

namespace learnChemistry::routing {

    class Router {
    public:
        void addRoute(http::verb method,
            const std::string& path,
            HandlerFn handler);

        std::optional<Route> match(http::verb method,
            const std::string& path) const;

    private:
        std::vector<Route> routes_;
    };

}
