#include <vector>
#include <string>
#include <iostream>
#include "SiteController.h"
#include "Website.h"

void SiteController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT title, subtitle, content, author, create_timestamp, tags FROM dwBlog WHERE isBlog=1 ORDER BY create_timestamp DESC LIMIT 3";
    auto result = clientPtr->execSqlSync(query);
    std::string content, title;
    for (auto row : result) {
        if (title.empty()) {
            title = row["title"].as<std::string>();
        }
        content.append(website::getContent(row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["author"].as<std::string>(), row["create_timestamp"].as<std::string>(), row["tags"].as<std::string>()));
        std::cout << content << std::endl;
    }
    auto site = new website(keywords, "en", title, content);
    callback(site->getPage());
}
