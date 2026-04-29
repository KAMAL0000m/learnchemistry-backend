#pragma once
#include <string>
#include <optional>

#include "db/MySqlPool.h"
#include "repositories/CourseRepository.h"
#include "models/Course.h"

namespace learnChemistry::services {

    class CourseService {
    public:
        explicit CourseService(learnChemistry::db::MySqlPool& pool);

        learnChemistry::repositories::CourseListResult listActiveCourses(int page, int limit, const std::string& search);
        std::optional<learnChemistry::models::Course> getActiveCourseById(long long courseId);

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::services