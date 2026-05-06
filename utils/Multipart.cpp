#include "Multipart.h"
#include <sstream>
#include <algorithm>

namespace learnChemistry::utils {

    static inline bool starts_with(const std::string& s, const std::string& p) {
        return s.size() >= p.size() && std::equal(p.begin(), p.end(), s.begin());
    }

    std::string Multipart::trim(const std::string& s) {
        size_t b = 0, e = s.size();
        while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) b++;
        while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) e--;
        return s.substr(b, e - b);
    }

    std::string Multipart::getBoundary(const std::string& contentTypeHeader) {
        // Example: multipart/form-data; boundary=----WebKitFormBoundaryabc123
        auto pos = contentTypeHeader.find("boundary=");
        if (pos == std::string::npos) return "";
        std::string b = contentTypeHeader.substr(pos + 9);
        b = trim(b);
        // boundary may be quoted
        if (!b.empty() && b.front() == '"' && b.back() == '"') b = b.substr(1, b.size() - 2);
        return b;
    }

    std::unordered_map<std::string, std::string> Multipart::parseHeaders(const std::string& headerBlock) {
        std::unordered_map<std::string, std::string> out;
        std::istringstream iss(headerBlock);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto colon = line.find(':');
            if (colon == std::string::npos) continue;
            std::string k = trim(line.substr(0, colon));
            std::string v = trim(line.substr(colon + 1));
            // normalize lowercase key optional; keep as-is for now
            out[k] = v;
        }
        return out;
    }

    void Multipart::parseContentDisposition(const std::string& value, std::string& nameOut, std::string& filenameOut) {
        // Example: form-data; name="pdf"; filename="abc.pdf"
        nameOut.clear();
        filenameOut.clear();

        // Find name=""
        auto npos = value.find("name=");
        if (npos != std::string::npos) {
            auto start = value.find('"', npos);
            auto end = (start != std::string::npos) ? value.find('"', start + 1) : std::string::npos;
            if (start != std::string::npos && end != std::string::npos) {
                nameOut = value.substr(start + 1, end - start - 1);
            }
        }

        auto fpos = value.find("filename=");
        if (fpos != std::string::npos) {
            auto start = value.find('"', fpos);
            auto end = (start != std::string::npos) ? value.find('"', start + 1) : std::string::npos;
            if (start != std::string::npos && end != std::string::npos) {
                filenameOut = value.substr(start + 1, end - start - 1);
            }
        }
    }

    std::vector<MultipartPart> Multipart::parse(const std::string& contentTypeHeader,
        const std::string& body) {
        std::vector<MultipartPart> parts;

        std::string boundary = getBoundary(contentTypeHeader);
        if (boundary.empty()) return parts;

        std::string delim = "--" + boundary;
        std::string endDelim = delim + "--";

        size_t pos = 0;

        // Find first boundary
        pos = body.find(delim, pos);
        if (pos == std::string::npos) return parts;

        while (true) {
            // Move after boundary line
            pos += delim.size();

            // If end boundary
            if (pos + 2 <= body.size() && body.compare(pos, 2, "--") == 0) break;

            // Skip CRLF after boundary
            if (pos + 2 <= body.size() && body.compare(pos, 2, "\r\n") == 0) pos += 2;

            // Headers end at \r\n\r\n
            size_t headerEnd = body.find("\r\n\r\n", pos);
            if (headerEnd == std::string::npos) break;

            std::string headerBlock = body.substr(pos, headerEnd - pos);
            auto headers = parseHeaders(headerBlock);

            pos = headerEnd + 4; // move to data start

            // Data ends at next boundary delimiter preceded by \r\n
            size_t next = body.find("\r\n" + delim, pos);
            if (next == std::string::npos) break;

            std::string data = body.substr(pos, next - pos);

            MultipartPart part;
            part.headers = headers;
            part.data = std::move(data);

            // Extract name/filename
            auto it = headers.find("Content-Disposition");
            if (it != headers.end()) {
                parseContentDisposition(it->second, part.name, part.filename);
            }
            auto ct = headers.find("Content-Type");
            if (ct != headers.end()) {
                part.contentType = trim(ct->second);
            }

            parts.push_back(std::move(part));
            pos = next + 2; // points at delim start next loop uses find; keep consistent
            pos = body.find(delim, pos);
            if (pos == std::string::npos) break;
        }

        return parts;
    }

} // namespace learnChemistry::utils