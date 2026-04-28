#include "services/AuthService.h"
#include "repositories/UserRepository.h"
#include "security/JwtService.h"
#include <stdexcept>

namespace learnChemistry::services {

    AuthService::AuthService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    LoginResult AuthService::login(const std::string& email,
        const std::string& password,
        const std::string& jwtSecret)
    {
        repositories::UserRepository repo(pool_);
        auto user = repo.findByEmail(email);

        if (!user || !hasher.verify(password, user->passwordHash)) {
            throw std::runtime_error("Invalid credentials");
        }

        const std::string token =
            security::JwtService::sign(user->id, user->role, jwtSecret);

        return LoginResult{ user->id, token };

    }

    SignupResult AuthService::signup(const std::string& email,
        const std::string& password,
        const std::string& jwtSecret)
    {
        repositories::UserRepository repo(pool_);  

        auto existingUser = repo.findByEmail(email);
        if (existingUser) {
            throw std::runtime_error("User already exists");
        }

        std::string passwordHash = hasher.hash(password);


        long long userId = repo.createUser(email, passwordHash, "USER");

        // no cast needed
        std::string token = security::JwtService::sign(userId, "USER", jwtSecret);

        return { userId, token };


    }

}
