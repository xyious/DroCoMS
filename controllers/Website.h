#pragma once
#include <drogon/HttpResponse.h>
#include <string>
#include <vector>

class website {
    private:
        std::vector<std::string>    keywords;
        std::string                 language;
        std::string                 title;
        std::string                 stylesheet;
        std::string                 content;
        std::string                 page;

    public:
        std::shared_ptr<drogon::HttpResponse>   getPage();
        std::string                             getTitle();
        std::string                             getTitleTag();
        std::string                             getStyleTag(std::string path);
        std::string                             getLanguague();
        
        website(std::vector<std::string> keywords, std::string languague, std::string title, std::string content, std::string stylesheet = "");
};