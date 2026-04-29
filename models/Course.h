#pragma once
#include <string>

namespace learnChemistry::models {

    struct Course {
        long long id = 0;
        std::string slug;
        std::string title;
        std::string description;
        long long pricePaise = 0;
        std::string currency = "INR";
        std::string thumbnailUrl;
        bool isActive = true;
    };

} // namespace learnChemistry::models