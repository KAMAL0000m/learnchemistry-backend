#include "repositories/CourseAssetRepository.h"

namespace learnChemistry::repositories {

    std::optional<PdfAssetInfo>
        CourseAssetRepository::findLatestActivePdf(mysqlx::Session& sess, long long courseId) {
        auto row = sess.sql(
            "SELECT id, course_id, storage_key, original_filename, mime_type, file_size "
            "FROM course_assets "
            "WHERE course_id=? AND asset_type='PDF' AND is_active=1 "
            "ORDER BY id DESC LIMIT 1"
        ).bind(courseId).execute().fetchOne();

        if (!row) return std::nullopt;

        PdfAssetInfo a;
        a.assetId = row[0].get<long long>();
        a.courseId = row[1].get<long long>();
        a.storageKey = row[2].get<std::string>();
        a.originalFilename = row[3].isNull() ? "" : row[3].get<std::string>();
        a.mimeType = row[4].isNull() ? "application/pdf" : row[4].get<std::string>();
        a.fileSize = row[5].get<long long>();
        return a;
    }

} // namespace learnChemistry::repositories