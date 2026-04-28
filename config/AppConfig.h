
#pragma once
#include <string>

namespace learnChemistry::config {

    struct AppConfig {
        std::string host = "0.0.0.0";
        unsigned short port = 8080;
        std::string jwt_secret;
        std::string db_host = "127.0.0.1";
        unsigned short db_port = 33060;  
        std::string db_user = "root";
        std::string db_password = "Jhargram@900";
        std::string db_name = "learnchemistry";
        int db_pool_size = 4;


    };

}
