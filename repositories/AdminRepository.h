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

        static long long insertCourseAsset(mysqlx::Session& sess,
            long long courseId,
            const std::string& assetType,
            const std::string& title,
            const std::string& storageKey,
            const std::string& originalFilename,
            const std::string& mimeType,
            long long fileSize,
            int isActive);

        static mysqlx::RowResult listOrders(mysqlx::Session& sess);
    };

} // namespace learnChemistry::repositories
