#include "services/MeService.h"
#include "repositories/EnrollmentRepository.h"

namespace learnChemistry::services {

    MeService::MeService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    nlohmann::json MeService::myCourses(long long userId) {
        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        auto rows = learnChemistry::repositories::EnrollmentRepository::findMyCourses(sess, userId);

        nlohmann::json items = nlohmann::json::array();
        for (mysqlx::Row r : rows) {
            items.push_back({
                {"id", r[0].get<long long>()},
                {"slug", r[1].get<std::string>()},
                {"title", r[2].get<std::string>()},
                {"description", r[3].isNull() ? "" : r[3].get<std::string>()},
                {"pricePaise", r[4].get<long long>()},
                {"currency", r[5].get<std::string>()},
                {"thumbnailUrl", r[6].isNull() ? "" : r[6].get<std::string>()},
                {"enrolledAt", r[7].get<std::string>()}
                });
        }

        return { {"items", items} };
    }

} // namespace learnChemistry::services