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

        // thumbnail_url will be filled after thumbnail upload; keep empty for now
        return learnChemistry::repositories::AdminRepository::insertCourse(
            sess, slug, title, description, pricePaise, "INR", /*thumbnailUrl*/"", /*active*/1
        );
    }

    PdfStoreResult AdminService::storePdf(long long courseId,
        const std::string& fileBytes,
        const std::string& originalFilename,
        const std::string& mimeType)
    {
        if (courseId <= 0) throw std::runtime_error("Invalid courseId");
        if (fileBytes.empty()) throw std::runtime_error("Empty PDF body");

        if (mimeType.find("application/pdf") == std::string::npos) {
            throw std::runtime_error("PDF Content-Type must be application/pdf");
        }

        const size_t MAX_BYTES = 50 * 1024 * 1024; // 50MB
        if (fileBytes.size() > MAX_BYTES) throw std::runtime_error("PDF too large (max 50MB)");

        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        if (!learnChemistry::repositories::AdminRepository::courseExists(sess, courseId)) {
            throw std::runtime_error("Course not found");
        }

        std::filesystem::create_directories("storage/pdfs");

        const std::string pdfPath =
            "storage/pdfs/course_" + std::to_string(courseId) + "_" + std::to_string(std::time(nullptr)) + ".pdf";

        std::ofstream out(pdfPath, std::ios::binary);
        out.write(fileBytes.data(), static_cast<std::streamsize>(fileBytes.size()));
        out.close();
        if (!out) throw std::runtime_error("Failed to write PDF to disk");

        learnChemistry::repositories::AdminRepository::updateCoursePdf(sess, courseId, pdfPath, mimeType);

        return { courseId, pdfPath };
    }

    static std::string extFromImageMime(const std::string& mime) {
        if (mime == "image/png") return ".png";
        if (mime == "image/jpeg") return ".jpg";
        if (mime == "image/webp") return ".webp";
        // default
        return ".jpg";
    }

    ThumbStoreResult AdminService::storeThumbnail(long long courseId,
        const std::string& fileBytes,
        const std::string& originalFilename,
        const std::string& mimeType)
    {
        if (courseId <= 0) throw std::runtime_error("Invalid courseId");
        if (fileBytes.empty()) throw std::runtime_error("Empty thumbnail body");

        if (mimeType.rfind("image/", 0) != 0) {
            throw std::runtime_error("Thumbnail Content-Type must be image/*");
        }

        const size_t MAX_BYTES = 5 * 1024 * 1024; // 5MB
        if (fileBytes.size() > MAX_BYTES) throw std::runtime_error("Thumbnail too large (max 5MB)");

        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        if (!learnChemistry::repositories::AdminRepository::courseExists(sess, courseId)) {
            throw std::runtime_error("Course not found");
        }

        std::filesystem::create_directories("storage/thumbs");

        const std::string ext = extFromImageMime(mimeType);
        const std::string thumbPath =
            "storage/thumbs/course_" + std::to_string(courseId) + "_" + std::to_string(std::time(nullptr)) + ext;

        std::ofstream out(thumbPath, std::ios::binary);
        out.write(fileBytes.data(), static_cast<std::streamsize>(fileBytes.size()));
        out.close();
        if (!out) throw std::runtime_error("Failed to write thumbnail to disk");

        const std::string publicUrl = "/v1/thumb/" + std::to_string(courseId);

        learnChemistry::repositories::AdminRepository::updateCourseThumbnail(sess, courseId, thumbPath, mimeType, publicUrl);

        return { courseId, thumbPath, publicUrl };
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