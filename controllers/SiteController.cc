#include <vector>
#include <string>
#include "SiteController.h"
#include "Website.h"

void SiteController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT dwBlog.title, dwBlog.subtitle, dwBlog.content, dwUsers.name, dwBlog.create_timestamp, dwBlog.tags FROM dwBlog, dwUsers WHERE dwBlog.author = dwUsers.id AND isBlog=1 ORDER BY create_timestamp DESC LIMIT 3";
    auto result = clientPtr->execSqlSync(query);
    std::string content, title;
    for (auto row : result) {
        if (title.empty()) {
            title = row["title"].as<std::string>();
        }
        content.append(website::getPost(row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["create_timestamp"].as<std::string>(), row["tags"].as<std::string>()));
    }
    auto site = new website(keywords, "en", title, content);
    callback(site->getPage());
}
