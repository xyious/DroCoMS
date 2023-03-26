#include <vector>
#include <drogon/HttpResponse.h>

class helpers {
    public:
        static std::string     BaseURL;

        static std::vector<std::string> split(std::string input, std::string delimiter) {
            std::vector<std::string> result;
            std::string token;
            while (!input.empty()) {
                size_t pos = input.find(delimiter);
                if (pos == std::string::npos) {
                    token = input;
                    input.erase(0, pos);
                } else {
                    token = input.substr(0, pos);
                    input.erase(0, pos + 1);
                }
                if (token.find(" ") == 0) {
                    token = token.substr(1);
                }
                result.push_back(token);
            }
            return result;
        }
};