#include "controllers/OrderController.h"

#include "services/OrderService.h"
#include "utils/HttpResponse.h"
#include <nlohmann/json.hpp>

namespace learnChemistry::controllers {

    OrderController::OrderController(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    http::response<http::string_body>
        OrderController::freePurchase(const http::request<http::string_body>& req,
            learnChemistry::context::RequestContext& ctx) const
    {
        using learnChemistry::utils::HttpResponse;

        if (!ctx.user.has_value()) {
            return HttpResponse::json(http::status::unauthorized, { {"error","Unauthorized"} });
        }

        nlohmann::json body = nlohmann::json::parse(req.body(), nullptr, false);
        if (body.is_discarded() || !body.is_object()) {
            return HttpResponse::json(http::status::bad_request, { {"error","Invalid JSON"} });
        }

        std::vector<long long> courseIds;

        // Accept either {courseId: X} OR {items:[{id:X,qty:Y},...]}
        if (body.contains("courseId")) {
            courseIds.push_back(body.value("courseId", 0LL));
        }
        else if (body.contains("items") && body["items"].is_array()) {
            for (auto& it : body["items"]) {
                long long id = it.value("id", 0LL);
                if (id > 0) courseIds.push_back(id);
            }
        }

        try {
            learnChemistry::services::OrderService svc(pool_);
            auto result = svc.freePurchase(ctx.user->id, courseIds);

            return HttpResponse::json(http::status::created, {
                {"orderId", result.orderId},
                {"enrolled", result.enrolled}
                });
        }
        catch (const std::exception& ex) {
            // Map known cases
            std::string msg = ex.what();
            if (msg == "Already enrolled") {
                return HttpResponse::json(http::status::conflict, { {"error", msg} });
            }
            if (msg == "Course not found") {
                return HttpResponse::json(http::status::not_found, { {"error", msg} });
            }
            return HttpResponse::json(http::status::bad_request, { {"error", msg} });
        }
    }

} // namespace learnChemistry::controllers