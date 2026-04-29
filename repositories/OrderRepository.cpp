#include "repositories/OrderRepository.h"
#include <stdexcept>

namespace learnChemistry::repositories {

    long long OrderRepository::createPaidOrder(mysqlx::Session& sess, long long userId, long long totalPaise, const std::string& currency) {
        sess.sql(
            "INSERT INTO orders (user_id, status, total_paise, currency, payment_provider, paid_at) "
            "VALUES (?, 'PAID', ?, ?, 'FREE', NOW())"
        ).bind(userId, totalPaise, currency).execute();

        auto r = sess.sql("SELECT LAST_INSERT_ID()").execute().fetchOne();
        if (!r) throw std::runtime_error("Failed to read order id");
        return r[0].get<long long>();
    }

    void OrderRepository::addOrderItem(mysqlx::Session& sess, long long orderId, long long courseId, long long pricePaise, const std::string& currency) {
        sess.sql(
            "INSERT INTO order_items (order_id, course_id, price_paise, currency) VALUES (?, ?, ?, ?)"
        ).bind(orderId, courseId, pricePaise, currency).execute();
    }

} // namespace learnChemistry::repositoriess