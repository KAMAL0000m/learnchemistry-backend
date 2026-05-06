#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "db/MySqlPool.h"

namespace learnChemistry::services {

    struct PdfStoreResult {
        long long courseId = 0;
        std::string pdfPath;
    };

    struct ThumbStoreResult {
        long long courseId = 0;
        std::string thumbPath;
        std::string thumbUrl; // public url (/v1/thumb/<id>)
    };

    class AdminService {
    public:
        explicit AdminService(learnChemistry::db::MySqlPool& pool);

        long long createCourse(const std::string& title,
            const std::string& exam,
            int priceInr,
            const std::string& description);

        PdfStoreResult storePdf(long long courseId,
            const std::string& fileBytes,
            const std::string& originalFilename,
            const std::string& mimeType);

        ThumbStoreResult storeThumbnail(long long courseId,
            const std::string& fileBytes,
            const std::string& originalFilename,
            const std::string& mimeType);

        nlohmann::json listOrders();

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::services