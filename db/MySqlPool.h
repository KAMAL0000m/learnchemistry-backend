#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include <mysqlx/xdevapi.h>

#include "config/AppConfig.h"

namespace learnChemistry::db {

    // mysqlx::Session is NOT thread-safe; pool gives one session per request/thread. [3](https://dev.mysql.com/doc/dev/connector-cpp/latest/devapi_example.html)
    class MySqlPool {
    public:
        MySqlPool();
        ~MySqlPool();

        void init(const learnChemistry::config::AppConfig& cfg);

        // Acquire a session (exclusive). Returned shared_ptr returns to pool on destruction.
        std::shared_ptr<mysqlx::Session> acquire();

    private:
        std::shared_ptr<mysqlx::Session> createSession_();

    private:
        bool initialized_ = false;

        std::string host_;
        unsigned short port_ = 33060;
        std::string user_;
        std::string password_;
        std::string database_;
        int poolSize_ = 4;

        std::mutex mtx_;
        std::condition_variable cv_;
        std::queue<std::shared_ptr<mysqlx::Session>> pool_;
    };

} // namespace learnChemistry::db