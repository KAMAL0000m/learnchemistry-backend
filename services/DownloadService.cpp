#include "services/DownloadService.h"
#include "repositories/EnrollmentRepository.h"

namespace learnChemistry::services {

    DownloadService::DownloadService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    std::optional<DownloadInfo>
        DownloadService::getPdfForUser(long long userId, long long courseId) {
        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        // must be enrolled
        if (!learnChemistry::repositories::EnrollmentRepository::isEnrolled(sess, userId, courseId)) {
            return std::nullopt;
        }

        auto assetOpt = learnChemistry::repositories::CourseAssetRepository::findLatestActivePdf(sess, courseId);
        if (!assetOpt) return std::nullopt;

        DownloadInfo info;
        info.filePath = assetOpt->storageKey;

        // choose filename
        info.filename = assetOpt->originalFilename.empty()
            ? ("course_" + std::to_string(courseId) + ".pdf")
            : assetOpt->originalFilename;

        info.mimeType = assetOpt->mimeType.empty() ? "application/pdf" : assetOpt->mimeType;
        return info;
    }

} // namespace learnChemistry::services