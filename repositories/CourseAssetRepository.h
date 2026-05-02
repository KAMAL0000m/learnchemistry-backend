#pragma once
#include <optional>
#include <string>
#include <mysqlx/xdevapi.h>

namespace learnChemistry::repositories {

    struct PdfAssetInfo {
        long long assetId = 0;
        long long courseId = 0;
        std::string storageKey;
        std::string originalFilename;
        std::string mimeType;
        long long fileSize = 0;
    };

    class CourseAssetRepository {
    public:
        static std::optional<PdfAssetInfo> findLatestActivePdf(mysqlx::Session& sess, long long courseId);
    };

} // namespace learnChemistry::repositories