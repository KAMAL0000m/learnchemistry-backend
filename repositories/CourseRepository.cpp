#include "repositories/CourseRepository.h"

#include <mysqlx/xdevapi.h>
#include <stdexcept>

namespace learnChemistry::repositories {

    CourseRepository::CourseRepository(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    static learnChemistry::models::Course rowToCourse(const mysqlx::Row& r) {
        learnChemistry::models::Course c;
        c.id = r[0].get<long long>();
        c.slug = r[1].get<std::string>();
        c.title = r[2].get<std::string>();
        c.description = r[3].isNull() ? "" : r[3].get<std::string>();
        c.pricePaise = r[4].get<long long>();
        c.currency = r[5].get<std::string>();
        c.thumbnailUrl = r[6].isNull() ? "" : r[6].get<std::string>();
        c.isActive = (r[7].get<int>() != 0);
        return c;
    }

    CourseListResult CourseRepository::findAllActive(int page, int limit, const std::string& search) {
        auto sess = pool_.acquire();

        const int safePage = (page < 1) ? 1 : page;
        const int safeLimit = (limit < 1) ? 12 : (limit > 100 ? 100 : limit);
        const int offset = (safePage - 1) * safeLimit;

        CourseListResult out;

        // Total count (for pagination)
        if (search.empty()) {
            auto countRes = sess->sql("SELECT COUNT(*) FROM courses WHERE is_active=1").execute();
            auto row = countRes.fetchOne();
            out.total = row ? row[0].get<long long>() : 0;
        }
        else {
            std::string like = "%" + search + "%";
            auto countRes = sess->sql(
                "SELECT COUNT(*) FROM courses WHERE is_active=1 AND (title LIKE ? OR slug LIKE ?)"
            ).bind(like, like).execute();
            auto row = countRes.fetchOne();
            out.total = row ? row[0].get<long long>() : 0;
        }

        // Items
        if (search.empty()) {
            auto res = sess->sql(
                "SELECT id, slug, title, description, price_paise, currency, thumbnail_url, is_active "
                "FROM courses WHERE is_active=1 "
                "ORDER BY id DESC LIMIT ? OFFSET ?"
            ).bind(safeLimit, offset).execute();

            for (mysqlx::Row row : res) {
                out.items.push_back(rowToCourse(row));
            }
        }
        else {
            std::string like = "%" + search + "%";
            auto res = sess->sql(
                "SELECT id, slug, title, description, price_paise, currency, thumbnail_url, is_active "
                "FROM courses WHERE is_active=1 AND (title LIKE ? OR slug LIKE ?) "
                "ORDER BY id DESC LIMIT ? OFFSET ?"
            ).bind(like, like, safeLimit, offset).execute();

            for (mysqlx::Row row : res) {
                out.items.push_back(rowToCourse(row));
            }
        }

        return out;
    }

    std::optional<learnChemistry::models::Course>
        CourseRepository::findActiveById(long long courseId) {
        auto sess = pool_.acquire();

        auto res = sess->sql(
            "SELECT id, slug, title, description, price_paise, currency, thumbnail_url, is_active "
            "FROM courses WHERE id=? AND is_active=1 LIMIT 1"
        ).bind(courseId).execute();

        mysqlx::Row row = res.fetchOne();
        if (!row) return std::nullopt;

        return rowToCourse(row);
    }

} // namespace learnChemistry::repositories