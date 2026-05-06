#pragma once
#include <string>
#include <mysqlx/xdevapi.h>

namespace learnChemistry::repositories {

    class AdminRepository {
    public:
        static bool slugExists(mysqlx::Session& sess, const std::string& slug);
        static bool courseExists(mysqlx::Session& sess, long long courseId);

        static long long insertCourse(mysqlx::Session& sess,
            const std::string& slug,
            const std::string& title,
            const std::string& description,
            long long pricePaise,
            const std::string& currency,
            const std::string& thumbnailUrl,
            int isActive);

        // ✅ NEW: update file paths stored directly in courses table
        static void updateCoursePdf(mysqlx::Session& sess, long long courseId,
            const std::string& pdfPath, const std::string& pdfMime);

        static void updateCourseThumbnail(mysqlx::Session& sess, long long courseId,
            const std::string& thumbPath, const std::string& thumbMime,
            const std::string& thumbUrl);

        static mysqlx::RowResult listOrders(mysqlx::Session& sess);
    };

} // namespace learnChemistry::repositories