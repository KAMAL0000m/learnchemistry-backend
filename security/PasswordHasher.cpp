#include "PasswordHasher.h"
namespace learnChemistry::security {
    bool PasswordHasher::verify(const std::string& password, const std::string& hash) {
        return crypto_pwhash_str_verify(
            hash.c_str(),
            password.c_str(),
            password.size()
        ) == 0;
    }

    std::string PasswordHasher::hash(const std::string& password) {
        char hash[crypto_pwhash_STRBYTES];

        crypto_pwhash_str(
            hash,
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE
        );

        return std::string(hash);
    }

}