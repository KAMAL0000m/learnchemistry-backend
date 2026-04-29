
#include "routing/Router.h"

namespace learnChemistry::routing {

    void Router::addRoute(http::verb method,
        const std::string& path,
        HandlerFn handler) {
        routes_.push_back({ method, path, std::move(handler) });
    }

    std::optional<Route>
        Router::match(http::verb method, const std::string& path) const
    {
        // ---- normalize incoming path ----
        std::string reqPath = path;

        // strip query string
        if (auto q = reqPath.find('?'); q != std::string::npos)
            reqPath.resize(q);

        // remove trailing slash (except root)
        if (reqPath.size() > 1 && reqPath.back() == '/')
            reqPath.pop_back();

        // helper: digit check
        auto isDigits = [](const std::string& s) {
            if (s.empty()) return false;
            for (char c : s) if (c < '0' || c > '9') return false;
            return true;
            };

        // helper: route match supports exact + "/:id" at end
        auto matchPath = [&](const std::string& routePath) -> bool {
            if (routePath == reqPath) return true;

            const std::string suffix = "/:id";
            if (routePath.size() > suffix.size() &&
                routePath.compare(routePath.size() - suffix.size(), suffix.size(), suffix) == 0)
            {
                const std::string prefix = routePath.substr(0, routePath.size() - suffix.size());

                // reqPath must start with "prefix/"
                if (reqPath.rfind(prefix + "/", 0) != 0) return false;

                const std::string idPart = reqPath.substr(prefix.size() + 1);
                return isDigits(idPart);
            }

            return false;
            };

        // ---- match ----
        for (const auto& r : routes_) {
            if (r.method == method && matchPath(r.path))
                return r;
        }
        return std::nullopt;
    }


}
