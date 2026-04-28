#pragma once
#include <string>
#include "security/PasswordHasher.h"
#include "db/MySqlPool.h"

namespace learnChemistry::services {

    struct LoginResult {
        long long userId;
        std::string token;
    };
    
    struct SignupResult {
        long long userId;
        std::string token;
    };

    class AuthService {
    public:
        explicit AuthService(learnChemistry::db::MySqlPool& pool);

        LoginResult login(const std::string& email,
            const std::string& password,
            const std::string& jwtSecret);

        SignupResult signup(const std::string& email,
            const std::string& password,
            const std::string& jwtSecret);
    private:
        security::PasswordHasher hasher;
        learnChemistry::db::MySqlPool& pool_;

    };



}
