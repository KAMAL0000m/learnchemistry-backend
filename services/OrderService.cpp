#include "services/OrderService.h"

#include "repositories/OrderRepository.h"
#include "repositories/EnrollmentRepository.h"

#include <stdexcept>

namespace learnChemistry::services {

    OrderService::OrderService(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    FreeOrderResult OrderService::freePurchase(long long userId, const std::vector<long long>& courseIds) {
        if (courseIds.empty()) throw std::runtime_error("No courseId/items provided");

        auto sessPtr = pool_.acquire();
        mysqlx::Session& sess = *sessPtr;

        sess.startTransaction();
        try {
            // Validate: course exists + not already enrolled
            long long totalPaise = 0;
            std::string currency = "INR";

            for (auto cid : courseIds) {
                if (cid <= 0) throw std::runtime_error("Invalid courseId");

                // check not already purchased
                if (learnChemistry::repositories::EnrollmentRepository::isEnrolled(sess, userId, cid)) {
                    throw std::runtime_error("Already enrolled");
                }

                // verify course exists and is active
                auto row = sess.sql(
                    "SELECT price_paise, currency FROM courses WHERE id=? AND is_active=1 LIMIT 1"
                ).bind(cid).execute().fetchOne();

                if (!row) throw std::runtime_error("Course not found");

                totalPaise += row[0].get<long long>();
                currency = row[1].get<std::string>();
            }

            // Create PAID order (free MVP)
            long long orderId = learnChemistry::repositories::OrderRepository::createPaidOrder(sess, userId, totalPaise, currency);

            // Add items + enrollments
            for (auto cid : courseIds) {
                auto row = sess.sql("SELECT price_paise, currency FROM courses WHERE id=? LIMIT 1")
                    .bind(cid).execute().fetchOne();

                const long long pricePaise = row ? row[0].get<long long>() : 0;
                const std::string cur = row ? row[1].get<std::string>() : "INR";

                learnChemistry::repositories::OrderRepository::addOrderItem(sess, orderId, cid, pricePaise, cur);
                learnChemistry::repositories::EnrollmentRepository::createEnrollment(sess, userId, cid, orderId);
            }

            sess.commit();
            return { orderId, true };
        }
        catch (...) {
            sess.rollback();
            throw;
        }
    }

} // namespace learnChemistry::services