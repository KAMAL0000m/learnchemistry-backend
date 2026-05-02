#pragma once
#include <string>
#include <optional>

#include "db/MySqlPool.h"
#include "repositories/CourseAssetRepository.h"

namespace learnChemistry::services {

    struct DownloadInfo {
        std::string filePath;
        std::string filename;
        std::string mimeType;
    };

    class DownloadService {
    public:
        explicit DownloadService(learnChemistry::db::MySqlPool& pool);

        // validates enrollment and returns file path + filename
        std::optional<DownloadInfo> getPdfForUser(long long userId, long long courseId);

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::services
