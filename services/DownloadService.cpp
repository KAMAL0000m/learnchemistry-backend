#include "services/DownloadService.h"
#include "repositories/EnrollmentRepository.h"

#include <mysqlx/xdevapi.h>

namespace learnChemistry::services {

    DownloadService::DownloadService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    std::optional<DownloadInfo>
        DownloadService::getPdfForUser(long long userId, long long courseId)
    {
        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        // must be enrolled
        if (!learnChemistry::repositories::EnrollmentRepository::isEnrolled(sess, userId, courseId)) {
            return std::nullopt;
        }

        // ✅ Read PDF info directly from courses table (your schema)
        auto row = sess.sql(
            "SELECT pdf_path, pdf_mime "
            "FROM courses "
            "WHERE id=? AND is_active=1 "
            "LIMIT 1"
        ).bind(courseId).execute().fetchOne();

        if (!row) return std::nullopt;
        if (row[0].isNull()) return std::nullopt; // pdf_path missing

        DownloadInfo info;
        info.filePath = row[0].get<std::string>();
        info.mimeType = row[1].isNull() ? "application/pdf" : row[1].get<std::string>();

        // filename (you don't store original pdf filename in courses currently)
        info.filename = "course_" + std::to_string(courseId) + ".pdf";

        return info;
    }

} // namespace learnChemistry::services