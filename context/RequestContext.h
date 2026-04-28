
#pragma once
#include <optional>
#include <string>

namespace learnChemistry::context {

    struct AuthUser {
        long long id{};
        std::string role;
    };

    struct RequestContext {
        std::optional<AuthUser> user;
    };

}
