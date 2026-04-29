#include "repositories/EnrollmentRepository.h"

namespace learnChemistry::repositories {

    bool EnrollmentRepository::isEnrolled(mysqlx::Session& sess, long long userId, long long courseId) {
        auto row = sess.sql(
            "SELECT 1 FROM enrollments WHERE user_id=? AND course_id=? LIMIT 1"
        ).bind(userId, courseId).execute().fetchOne();
        return static_cast<bool>(row);
    }

    void EnrollmentRepository::createEnrollment(mysqlx::Session& sess, long long userId, long long courseId, long long orderId) {
        sess.sql(
            "INSERT INTO enrollments (user_id, course_id, order_id) VALUES (?, ?, ?)"
        ).bind(userId, courseId, orderId).execute();
    }

    mysqlx::RowResult EnrollmentRepository::findMyCourses(mysqlx::Session& sess, long long userId) {
        return sess.sql(
            "SELECT c.id, c.slug, c.title, c.description, c.price_paise, c.currency, c.thumbnail_url, e.enrolled_at "
            "FROM enrollments e "
            "JOIN courses c ON c.id = e.course_id "
            "WHERE e.user_id = ? "
            "ORDER BY e.enrolled_at DESC"
        ).bind(userId).execute();
    }

} // namespace learnChemistry::repositories