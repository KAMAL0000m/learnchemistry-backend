// services/AdminService.cpp

#include "services/AdminService.h"
#include "repositories/AdminRepository.h"

#include <filesystem>
#include <fstream>
#include <ctime>
#include <stdexcept>

namespace learnChemistry::services {

    AdminService::AdminService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    static std::string slugify(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        bool dash = false;
        for (char ch : s) {
            if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
                out.push_back(ch);
                dash = false;
            }
            else if (ch >= 'A' && ch <= 'Z') {
                out.push_back(static_cast<char>(ch - 'A' + 'a'));
                dash = false;
            }
            else {
                if (!dash) out.push_back('-');
                dash = true;
            }
        }
        while (!out.empty() && out.back() == '-') out.pop_back();
        if (out.empty()) out = "course";
        return out;
    }

    long long AdminService::createCourse(const std::string& title,
        const std::string& exam,
        int priceInr,
        const std::string& description)
    {
        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        const long long pricePaise = static_cast<long long>(priceInr) * 100;

        std::string slug = slugify(title + "-" + exam);
        if (learnChemistry::repositories::AdminRepository::slugExists(sess, slug)) {
            slug += "-" + std::to_string(std::time(nullptr));
        }

        return learnChemistry::repositories::AdminRepository::insertCourse(
            sess, slug, title, description, pricePaise, "INR", /*thumbnail*/"", /*active*/1
        );
    }

    // ✅ FIX: return type is PdfStoreResult (NOT AdminService::PdfStoreResult)
    PdfStoreResult AdminService::storePdf(long long courseId,
        const std::string& fileBytes,
        const std::string& originalFilename,
        const std::string& mimeType)
    {
        if (courseId <= 0) throw std::runtime_error("Invalid courseId");
        if (fileBytes.empty()) throw std::runtime_error("Empty file body");

        // MVP safety limit (adjust as needed)
        const size_t MAX_BYTES = 25 * 1024 * 1024; // 25 MB
        if (fileBytes.size() > MAX_BYTES) throw std::runtime_error("PDF too large (max 25MB)");

        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        if (!learnChemistry::repositories::AdminRepository::courseExists(sess, courseId)) {
            throw std::runtime_error("Course not found");
        }

        std::filesystem::create_directories("storage/pdfs");

        const std::string storageKey =
            "storage/pdfs/course_" + std::to_string(courseId) + "_" + std::to_string(std::time(nullptr)) + ".pdf";

        std::ofstream out(storageKey, std::ios::binary);
        out.write(fileBytes.data(), static_cast<std::streamsize>(fileBytes.size()));
        out.close();

        if (!out) {
            throw std::runtime_error("Failed to write PDF to disk");
        }

        const long long fileSize = static_cast<long long>(fileBytes.size());

        long long assetId = learnChemistry::repositories::AdminRepository::insertCourseAsset(
            sess,
            courseId,
            "PDF",
            /*title*/"",
            storageKey,
            originalFilename,
            mimeType,
            fileSize,
            /*active*/1
        );

        return PdfStoreResult{ assetId, courseId, storageKey };
    }

    nlohmann::json AdminService::listOrders() {
        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        auto rows = learnChemistry::repositories::AdminRepository::listOrders(sess);

        nlohmann::json items = nlohmann::json::array();
        for (mysqlx::Row r : rows) {
            items.push_back({
                {"orderId", r[0].get<long long>()},
                {"email", r[1].isNull() ? "" : r[1].get<std::string>()},
                {"totalPaise", r[2].get<long long>()},
                {"currency", r[3].get<std::string>()},
                {"status", r[4].get<std::string>()},
                {"createdAt", r[5].get<std::string>()}
                });
        }

        return { {"items", items} };
    }

} // namespace learnChemistry::services
