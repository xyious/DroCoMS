#include <string>
#include <vector>

class website {
    private:
        std::vector<std::string>    keywords;
        std::string                 language;
        std::string                 title;
        std::string                 content;
        std::string                 page;

    public:
        std::string     getPage();
        std::string     getTitle();
        std::string     getLanguague();
        
        website(std::vector<std::string> keywords, std::string languague, std::string title, std::string content);
};