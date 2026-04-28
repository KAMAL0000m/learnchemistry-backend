#pragma once

#include <string>
#include <optional>
#include "context/RequestContext.h"

namespace learnChemistry::security {

    class JwtService {
    public:
        static std::string sign(long long uid, const std::string& role, const std::string& secret);
        static std::optional<learnChemistry::context::AuthUser>
            verify(const std::string& token, const std::string& secret);
    };

} // namespace learnChemistry::security
