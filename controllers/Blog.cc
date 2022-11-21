#include "Website.h"
#include "Blog.h"

void Blog::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    if (req->getMethod() == HttpMethod::Post) {
        std::string title, subtitle, tags, content, author, isBlog;
        auto params = req->getParameters();
        for (auto param : params) {
            std::string key = std::get<0>(param);
            std::string value = std::get<1>(param);
            if (key == "title") {
                title = value;
            } else if (key == "subtitle") {
                subtitle = value;
            } else if (key == "tags") {
                tags = value;
            } else if (key == "content") {
                content = value;
            } else if (key == "isBlog") {
                isBlog = value;
            }
        }
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        std::string query = "INSERT INTO dwblog (title, subtitle, content, isBlog, author) VALUES ($1, $2, $3, $4, $5)";
        auto result = clientPtr->execSqlSync(query, title, subtitle, content, isBlog, 0);       // TODO: obviously fix this, need to make isBlog and author dropdown once I figure out how to create more users
        auto site = new website(keywords, "en", "Creating Post !", "Creating Post !");
        callback(site->getPage());
        return;
    } else {
        std::string form = "<form action='' method='post'><label for='title'>Title:</label><input type='text' name='Title' required><br><label for='subtitle'>Subtitle:</label><input type='text' name='subtitle'><br>\
        <label for='author'>Author:</label><input type='text' name='author'><br><label for='isBlog'>isBlog ?:</label><input type='text' name='isBlog'><br>\
        <label for='content'>Content:</label><input type='textarea' name='content'><br><input type='submit' value='submit'><form>";
        auto site = new website(keywords, "en", "Create Post", form);
        callback(site->getPage());
        return;
    }
}
