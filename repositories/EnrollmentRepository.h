#pragma once
#include <vector>
#include <optional>
#include <mysqlx/xdevapi.h>

namespace learnChemistry::repositories {

    class EnrollmentRepository {
    public:
        static bool isEnrolled(mysqlx::Session& sess, long long userId, long long courseId);
        static void createEnrollment(mysqlx::Session& sess, long long userId, long long courseId, long long orderId);

        // For dashboard
        static mysqlx::RowResult findMyCourses(mysqlx::Session& sess, long long userId);
    };

} // namespace learnChemistry::repositories