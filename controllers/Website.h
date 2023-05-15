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
        void                                    setContent(std::string content);
        void                                    setLanguage(std::string language);
        void                                    setTitle(std::string title);
        void                                    setLeftSidebarContent(std::string content);
        void                                    setRightSidebarContent(std::string content);
        void                                    setKeywords(std::vector<std::string> keywords);
        std::string                             getLanguage();
        std::string                             getLeftSidebar();
        std::shared_ptr<drogon::HttpResponse>   getPage();
        static std::string                      getPost(std::string url, std::string title, std::string subtitle, std::string content, std::string author, std::string timestamp);
        std::string                             getRightSidebar();
        std::string                             getTitle();
        std::string                             getTitleTag();
        std::string                             getStyleTag(std::string path);
        
        website();
        website(std::vector<std::string> keywords, std::string language, std::string title, std::string content, std::string leftSidebarContent = "", std::string rightSidebarContent = "", std::string stylesheet = "");
};