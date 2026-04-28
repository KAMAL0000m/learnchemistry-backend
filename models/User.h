#pragma once
#include <string>
namespace learnChemistry::models {
struct User { long long id{}; std::string email; std::string passwordHash; std::string role; };
}
