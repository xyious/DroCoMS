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
        std::string                 leftSidebarContent;
        std::string                 rightSidebarContent;
        std::string                 page;

    public:
        std::string                             getLanguage();
        std::string                             getLeftSidebar();
        std::shared_ptr<drogon::HttpResponse>   getPage();
        static std::string                      getPost(std::string url, std::string title, std::string subtitle, std::string content, std::string author, std::string timestamp, std::vector<std::string> tags);
        std::string                             getRightSidebar();
        std::string                             getTitle();
        std::string                             getTitleTag();
        std::string                             getStyleTag(std::string path);
        
        website(std::vector<std::string> keywords, std::string languague, std::string title, std::string content, std::string leftSidebarContent = "", std::string rightSidebarContent = "", std::string stylesheet = "");
};