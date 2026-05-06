#include "repositories/AdminRepository.h"
#include <stdexcept>

namespace learnChemistry::repositories {

    bool AdminRepository::slugExists(mysqlx::Session& sess, const std::string& slug) {
        auto row = sess.sql("SELECT 1 FROM courses WHERE slug=? LIMIT 1")
            .bind(slug).execute().fetchOne();
        return static_cast<bool>(row);
    }

    bool AdminRepository::courseExists(mysqlx::Session& sess, long long courseId) {
        auto row = sess.sql("SELECT 1 FROM courses WHERE id=? LIMIT 1")
            .bind(courseId).execute().fetchOne();
        return static_cast<bool>(row);
    }

    long long AdminRepository::insertCourse(mysqlx::Session& sess,
        const std::string& slug,
        const std::string& title,
        const std::string& description,
        long long pricePaise,
        const std::string& currency,
        const std::string& thumbnailUrl,
        int isActive)
    {
        sess.sql(
            "INSERT INTO courses (slug, title, description, price_paise, currency, thumbnail_url, is_active) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)"
        ).bind(slug, title, description, pricePaise, currency, thumbnailUrl, isActive).execute();

        auto r = sess.sql("SELECT LAST_INSERT_ID()").execute().fetchOne();
        if (!r) throw std::runtime_error("AdminRepository::insertCourse: LAST_INSERT_ID() failed");
        return r[0].get<long long>();
    }

    void AdminRepository::updateCoursePdf(mysqlx::Session& sess, long long courseId,
        const std::string& pdfPath, const std::string& pdfMime)
    {
        sess.sql("UPDATE courses SET pdf_path=?, pdf_mime=? WHERE id=?")
            .bind(pdfPath, pdfMime, courseId)
            .execute();
    }

    void AdminRepository::updateCourseThumbnail(mysqlx::Session& sess, long long courseId,
        const std::string& thumbPath, const std::string& thumbMime,
        const std::string& thumbUrl)
    {
        sess.sql("UPDATE courses SET thumbnail_path=?, thumbnail_mime=?, thumbnail_url=? WHERE id=?")
            .bind(thumbPath, thumbMime, thumbUrl, courseId)
            .execute();
    }

    mysqlx::RowResult AdminRepository::listOrders(mysqlx::Session& sess) {
        return sess.sql(
            "SELECT o.id, u.email, o.total_paise, o.currency, o.status, o.created_at "
            "FROM orders o "
            "JOIN users u ON u.id = o.user_id "
            "ORDER BY o.id DESC LIMIT 200"
        ).execute();
    }

} // namespace learnChemistry::repositories