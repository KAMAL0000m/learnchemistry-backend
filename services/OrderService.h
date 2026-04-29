#pragma once
#include <vector>
#include <string>

#include "db/MySqlPool.h"

namespace learnChemistry::services {

    struct FreeOrderResult {
        long long orderId = 0;
        bool enrolled = false;
    };

    class OrderService {
    public:
        explicit OrderService(learnChemistry::db::MySqlPool& pool);

        FreeOrderResult freePurchase(long long userId, const std::vector<long long>& courseIds);

    private:
        learnChemistry::db::MySqlPool& pool_;
    };

} // namespace learnChemistry::services