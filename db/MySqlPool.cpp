#include "MySqlPool.h"
#include <stdexcept>
#include <utility>

namespace learnChemistry::db {

    MySqlPool::MySqlPool() = default;
    MySqlPool::~MySqlPool() = default;

    void MySqlPool::init(const learnChemistry::config::AppConfig& cfg) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (initialized_) return;

        host_ = cfg.db_host;
        port_ = cfg.db_port;
        user_ = cfg.db_user;
        password_ = cfg.db_password;
        database_ = cfg.db_name;
        poolSize_ = cfg.db_pool_size;

        for (int i = 0; i < poolSize_; ++i) {
            pool_.push(createSession_());
        }

        initialized_ = true;
    }

    std::shared_ptr<mysqlx::Session> MySqlPool::createSession_() 
    {

        mysqlx::SessionSettings settings(
            mysqlx::SessionOption::HOST, host_,
            mysqlx::SessionOption::PORT, port_,
            mysqlx::SessionOption::USER, user_,
            mysqlx::SessionOption::PWD, password_
        );

        auto sess = std::make_shared<mysqlx::Session>(settings);

        sess->sql("USE " + database_).execute();

        return sess;
    }

    std::shared_ptr<mysqlx::Session> MySqlPool::acquire() {
        std::unique_lock<std::mutex> lk(mtx_);
        if (!initialized_) {
            throw std::runtime_error("MySqlPool::init() must be called before acquire()");
        }

        cv_.wait(lk, [this] { return !pool_.empty(); });

        auto sess = pool_.front();
        pool_.pop();

        // Return session to pool automatically when shared_ptr goes out of scope
        return std::shared_ptr<mysqlx::Session>(
            sess.get(),
            [this, sess](mysqlx::Session*) mutable {
                std::lock_guard<std::mutex> g(mtx_);
                pool_.push(std::move(sess));
                cv_.notify_one();
            }
        );
    }

} // namespace learnChemistry::db