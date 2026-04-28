
#include "config/AppConfig.h"
#include "server/HttpServer.h"

int main() {
    learnChemistry::config::AppConfig cfg;
    cfg.host = "0.0.0.0";
    cfg.port = 8080;
    cfg.jwt_secret = "dev-secret";

    learnChemistry::server::HttpServer server(cfg);
    server.run();
    return 0;
}
