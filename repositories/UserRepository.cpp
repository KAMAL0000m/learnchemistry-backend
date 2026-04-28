#include "repositories/UserRepository.h"
#include <stdexcept>

namespace learnChemistry::repositories {

    UserRepository::UserRepository(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    std::optional<learnChemistry::models::User>
        UserRepository::findByEmail(const std::string& email) {
        auto sess = pool_.acquire();

        // X DevAPI can execute SQL too (not only document store). [3](https://dev.mysql.com/doc/dev/connector-cpp/latest/devapi_example.html)[1](https://dev.mysql.com/doc/dev/connector-cpp/latest/usage.html)
        mysqlx::RowResult result = sess->sql(
            "SELECT id, email, password_hash, role FROM users WHERE email = ? LIMIT 1"
        ).bind(email).execute();

        mysqlx::Row row = result.fetchOne();
        if (!row) {
            return std::nullopt;
        }

        learnChemistry::models::User u;
        u.id = row[0].get<int>();
        u.email = row[1].get<std::string>();
        u.passwordHash = row[2].get<std::string>();
        u.role = row[3].get<std::string>();

        return u;
    }

    long long UserRepository::createUser(const std::string& email,
        const std::string& passwordHash,
        const std::string& role) {
        auto sess = pool_.acquire();

        sess->sql(
            "INSERT INTO users (email, password_hash, role) VALUES (?, ?, ?)"
        ).bind(email, passwordHash, role).execute();

        mysqlx::RowResult idRes = sess->sql("SELECT LAST_INSERT_ID()").execute();
        mysqlx::Row idRow = idRes.fetchOne();
        if (!idRow) {
            throw std::runtime_error("Failed to read LAST_INSERT_ID()");
        }

        return idRow[0].get<long long>();
    }

} // namespace learnChemistry::repositories