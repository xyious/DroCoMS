#include <regex>
#include "Website.h"
#include "Blog.h"

void Blog::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    std::string title, subtitle, tags, content, author, isBlog = "0";
    if (req->getMethod() == HttpMethod::Post) {
        auto params = req->getParameters();
        for (auto param : params) {
            std::string key = std::get<0>(param);
            std::string value = std::get<1>(param);
            LOG_TRACE << "key: " << key << ", value: " << value;
            if (key == "title") {
                title = value;
            } else if (key == "subtitle") {
                subtitle = value;
            } else if (key == "tags") {
                tags = value;
            } else if (key == "content") {
                value = std::regex_replace(value, std::regex("\\r\\n\\r\\n"), "</p><br><p>");
                value = std::regex_replace(value, std::regex("\\r\\n"), "</p><p>");
                content = "<p>" + value + "</p>";
            } else if (key == "author") {
                author = value;
            } else if (key == "isBlog") {
                if (value == "on") {
                    isBlog = "1";
                }
            }
        }
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        std::string url = std::regex_replace(title, std::regex(" "), "-");
        url = std::regex_replace(url, std::regex("[^A-Za-z0-9-_]"), "");
        std::string query = "INSERT INTO dwblog (url, title, subtitle, content, isBlog, author) VALUES ($1, $2, $3, $4, $5, $6)";
        try {
            auto result = clientPtr->execSqlSync(query, url, title, subtitle, content, isBlog, author);
            LOG_TRACE << "Url: " << url << ", Title: " << title << ", Subtitle: " << subtitle << ", Author: " << author << ", Content: " << content;
            auto site = new website(keywords, "en", "Creating Post !", "Creating Post !");
            callback(site->getPage());
            return;
        }
        catch (std::exception const& e) {
            LOG_ERROR << "Exception in Blog.cc: " << e.what();
        }
    }
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT id, name FROM dwUsers";
    auto result = clientPtr->execSqlSync(query);
    std::string form = "<form action='' method='post'><label for='title'>Title:</label><input type='text' name='title' value='";
    form.append(title);
    form.append("' required><br><label for='subtitle'>Subtitle:</label><input type='text' name='subtitle' value='");
    form.append(subtitle);
    form.append("'><br><label for='author'>Author:</label><select name='author'>");
    for (auto row : result) {
        form.append("<option value='");
        form.append(row["id"].as<std::string>());
        form.append("'>");
        form.append(row["name"].as<std::string>());
        form.append("</option>");
    }
    form.append("</select><br><label for='isBlog'>isBlog ?:</label><input type='checkbox' id='isBlog' name='isBlog' checked><br><label for='content'>Content:</label><textarea name='content' cols='128' rows='50'>");
    form.append(content);
    form.append("</textarea><br><input type='submit' value='submit'><form>");
    auto site = new website(keywords, "en", "Create Post", form);
    callback(site->getPage());
    return;
}
