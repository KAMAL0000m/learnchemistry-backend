#pragma once
#include <string>
#include <mysqlx/xdevapi.h>

namespace learnChemistry::repositories {

    class OrderRepository {
    public:
        static long long createPaidOrder(mysqlx::Session& sess, long long userId, long long totalPaise, const std::string& currency);
        static void addOrderItem(mysqlx::Session& sess, long long orderId, long long courseId, long long pricePaise, const std::string& currency);
    };

} // namespace learnChemistry::repositories