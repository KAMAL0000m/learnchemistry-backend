#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace learnChemistry::utils {

    struct MultipartPart {
        std::unordered_map<std::string, std::string> headers;
        std::string name;       // form field name
        std::string filename;   // for file parts
        std::string contentType;
        std::string data;       // raw bytes stored in std::string (OK if written via .write)
        bool isFile() const { return !filename.empty(); }
    };

    class Multipart {
    public:
        // Parses multipart/form-data body using boundary extracted from contentTypeHeader
        static std::vector<MultipartPart> parse(const std::string& contentTypeHeader,
            const std::string& body);

        static std::string getBoundary(const std::string& contentTypeHeader);

    private:
        static std::string trim(const std::string& s);
        static std::unordered_map<std::string, std::string> parseHeaders(const std::string& headerBlock);
        static void parseContentDisposition(const std::string& value, std::string& nameOut, std::string& filenameOut);
    };

} // namespace learnChemistry::utils