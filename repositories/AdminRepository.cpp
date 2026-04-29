// repositories/AdminRepository.cpp

#include "repositories/AdminRepository.h"
#include <stdexcept>

namespace learnChemistry::repositories {

    bool AdminRepository::slugExists(mysqlx::Session& sess, const std::string& slug) {
        mysqlx::Row row = sess.sql(
            "SELECT 1 FROM courses WHERE slug = ? LIMIT 1"
        ).bind(slug).execute().fetchOne();

        return static_cast<bool>(row);
    }

    bool AdminRepository::courseExists(mysqlx::Session& sess, long long courseId) {
        mysqlx::Row row = sess.sql(
            "SELECT 1 FROM courses WHERE id = ? LIMIT 1"
        ).bind(courseId).execute().fetchOne();

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

        mysqlx::Row r = sess.sql("SELECT LAST_INSERT_ID()").execute().fetchOne();
        if (!r) {
            throw std::runtime_error("AdminRepository::insertCourse: failed to read LAST_INSERT_ID()");
        }
        return r[0].get<long long>();
    }

    long long AdminRepository::insertCourseAsset(mysqlx::Session& sess,
        long long courseId,
        const std::string& assetType,
        const std::string& title,
        const std::string& storageKey,
        const std::string& originalFilename,
        const std::string& mimeType,
        long long fileSize,
        int isActive)
    {
        sess.sql(
            "INSERT INTO course_assets "
            "(course_id, asset_type, title, storage_key, original_filename, mime_type, file_size, is_active) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
        ).bind(courseId, assetType, title, storageKey, originalFilename, mimeType, fileSize, isActive).execute();

        mysqlx::Row r = sess.sql("SELECT LAST_INSERT_ID()").execute().fetchOne();
        if (!r) {
            throw std::runtime_error("AdminRepository::insertCourseAsset: failed to read LAST_INSERT_ID()");
        }
        return r[0].get<long long>();
    }

    mysqlx::RowResult AdminRepository::listOrders(mysqlx::Session& sess) {
        // Basic admin view: orders joined with user email
        return sess.sql(
            "SELECT o.id, u.email, o.total_paise, o.currency, o.status, o.created_at "
            "FROM orders o "
            "JOIN users u ON u.id = o.user_id "
            "ORDER BY o.id DESC "
            "LIMIT 200"
        ).execute();
    }

} // namespace learnChemistry::repositories
