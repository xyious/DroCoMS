#include <iostream>
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
            std::cout << key << " " << value << std::endl;
            if (key == "title") {
                title = value;
            } else if (key == "subtitle") {
                subtitle = value;
            } else if (key == "tags") {
                tags = value;
            } else if (key == "content") {
                content = value;
            } else if (key == "author") {
                author = value;
            } else if (key == "isBlog") {
                if (value == "on") {
                    isBlog = "1";
                }
            }
        }
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        std::string query = "INSERT INTO dwblog (title, subtitle, content, isBlog, author) VALUES ($1, $2, $3, $4, $5)";
        try {
            auto result = clientPtr->execSqlSync(query, title, subtitle, content, isBlog, author);       // TODO: obviously fix this, need to make isBlog and author dropdown once I figure out how to create more users
            auto site = new website(keywords, "en", "Creating Post !", "Creating Post !");
            callback(site->getPage());
            return;
        }
        catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
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
