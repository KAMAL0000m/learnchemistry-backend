#pragma once
#include <string>
#include <vector>
#include <optional>

#include "db/MySqlPool.h"
#include "models/Course.h"

namespace learnChemistry::repositories {

    struct CourseListResult {
        std::vector<learnChemistry::models::Course> items;
        long long total = 0;
    };

    class CourseRepository {
    public:
        explicit CourseRepository(learnChemistry::db::MySqlPool& pool);

        CourseListResult findAllActive(int page, int limit, const std::string& search);
        std::optional<learnChemistry::models::Course> findActiveById(long long courseId);

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::repositories