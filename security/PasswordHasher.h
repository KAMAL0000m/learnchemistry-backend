#pragma once
#include <string>
#include <sodium.h>
namespace learnChemistry::security {
class PasswordHasher {
public:
    bool verify(const std::string& plain, const std::string& hash);
    std::string hash(const std::string& password);
};
}
