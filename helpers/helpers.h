#include <vector>

class helpers {
    public:
        static std::vector<std::string> split(std::string input, std::string delimiter) {
            std::vector<std::string> result;
            std::string token;
            while (input.length() > 0) {
                size_t pos = input.find(delimiter);
                if (pos == std::string::npos) {
                    token = input;
                    input.erase(0, input.length());
                } else {
                    token = input.substr(0, pos);
                }
                if (token.find(" ") == 0) {
                    token = token.substr(1);
                }
                result.push_back(token);
            }
            return result;
        }
};