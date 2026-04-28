#pragma once

#include <optional>
#include <string>

#include "models/User.h"
#include "db/MySqlPool.h"

namespace learnChemistry::repositories {

    class UserRepository {
    public:
        explicit UserRepository(learnChemistry::db::MySqlPool& pool);

        std::optional<learnChemistry::models::User> findByEmail(const std::string& email);

        // For signup
        long long createUser(const std::string& email,
            const std::string& passwordHash,
            const std::string& role = "USER");

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::repositories