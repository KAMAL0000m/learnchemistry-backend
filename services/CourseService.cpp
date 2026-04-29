#include "services/CourseService.h"

namespace learnChemistry::services {

    CourseService::CourseService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    learnChemistry::repositories::CourseListResult
        CourseService::listActiveCourses(int page, int limit, const std::string& search) {
        learnChemistry::repositories::CourseRepository repo(pool_);
        return repo.findAllActive(page, limit, search);
    }

    std::optional<learnChemistry::models::Course>
        CourseService::getActiveCourseById(long long courseId) {
        learnChemistry::repositories::CourseRepository repo(pool_);
        return repo.findActiveById(courseId);
    }

} // namespace learnChemistry::services