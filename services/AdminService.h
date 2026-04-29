#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "db/MySqlPool.h"

namespace learnChemistry::services {

    struct PdfStoreResult {
        long long assetId = 0;
        long long courseId = 0;
        std::string storageKey;
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

        nlohmann::json listOrders();

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::services
