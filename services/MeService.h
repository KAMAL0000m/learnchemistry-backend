#pragma once
#include "db/MySqlPool.h"
#include <nlohmann/json.hpp>

namespace learnChemistry::services {

    class MeService {
    public:
        explicit MeService(learnChemistry::db::MySqlPool& pool);

        nlohmann::json myCourses(long long userId);

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::services