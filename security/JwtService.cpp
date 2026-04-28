#include "JwtService.h"

#include <openssl/hmac.h>
#include <ctime>
#include <string>
#include <array>

namespace learnChemistry::security {

    static std::string hmacSha256Raw(const std::string& data, const std::string& key) {
        unsigned int len = 0;
        std::array<unsigned char, EVP_MAX_MD_SIZE> out{};

        HMAC(EVP_sha256(),
            key.data(), static_cast<int>(key.size()),
            reinterpret_cast<const unsigned char*>(data.data()), data.size(),
            out.data(), &len);

        return std::string(reinterpret_cast<const char*>(out.data()), len); // raw bytes
    }

    static std::string toHex(const std::string& raw) {
        static const char* hex = "0123456789abcdef";
        std::string out;
        out.reserve(raw.size() * 2);

        for (unsigned char c : raw) {
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
        return out;
    }

    std::string JwtService::sign(long long uid, const std::string& role, const std::string& secret) {
        const auto exp = std::time(nullptr) + 900; // 15 minutes

        // payload: uid:role:exp  (ASCII)
        const std::string payload =
            std::to_string(uid) + ":" + role + ":" + std::to_string(exp);

        // ✅ IMPORTANT: encode HMAC bytes to HEX (ASCII-safe)
        const std::string sigHex = toHex(hmacSha256Raw(payload, secret));

        return payload + "." + sigHex;
    }

    std::optional<learnChemistry::context::AuthUser>
        JwtService::verify(const std::string& token, const std::string& secret) {
        const auto dot = token.find('.');
        if (dot == std::string::npos) return std::nullopt;

        const std::string payload = token.substr(0, dot);
        const std::string sigHex = token.substr(dot + 1);

        // Recompute signature and compare in hex form (ASCII)
        const std::string expectedHex = toHex(hmacSha256Raw(payload, secret));
        if (expectedHex != sigHex) return std::nullopt;

        // Parse payload robustly using first ':' and last ':'
        const auto first = payload.find(':');
        const auto last = payload.rfind(':');

        if (first == std::string::npos || last == std::string::npos || first == last)
            return std::nullopt;

        const std::string uidStr = payload.substr(0, first);
        const std::string role = payload.substr(first + 1, last - first - 1);
        const std::string expStr = payload.substr(last + 1);

        long long uid = 0;
        long long exp = 0;

        try {
            uid = std::stoll(uidStr);
            exp = std::stoll(expStr);
        }
        catch (...) {
            return std::nullopt;
        }

        if (std::time(nullptr) > exp) return std::nullopt;

        return learnChemistry::context::AuthUser{ uid, role };
    }

} // namespace learnChemistry::security
