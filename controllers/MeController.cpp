#include "controllers/MeController.h"

#include "services/MeService.h"
#include "utils/HttpResponse.h"

namespace learnChemistry::controllers {

    MeController::MeController(learnChemistry::db::MySqlPool& pool)
        : pool_(pool) {
    }

    http::response<http::string_body>
        MeController::myCourses(const http::request<http::string_body>&,
            learnChemistry::context::RequestContext& ctx) const
    {
        using learnChemistry::utils::HttpResponse;

        if (!ctx.user.has_value()) {
            return HttpResponse::json(http::status::unauthorized, { {"error","Unauthorized"} });
        }

        learnChemistry::services::MeService svc(pool_);
        auto body = svc.myCourses(ctx.user->id);

        return HttpResponse::json(http::status::ok, body);
    }

} // namespace learnChemistry::controllers