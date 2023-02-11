#include <regex>
#include <map>
#include "helpers/helpers.h"
#include "Website.h"
#include "Blog.h"

void Blog::create(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    std::string title, subtitle, tags, content, author, isBlog = "0";
    if (req->getMethod() == drogon::HttpMethod::Post) {
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
        keywords = helpers::split(tags, ",");
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        std::string url = std::regex_replace(title, std::regex(" "), "-");
        url = std::regex_replace(url, std::regex("[^A-Za-z0-9-_]"), "");
        std::string query = "INSERT INTO dwblog (url, title, subtitle, content, isBlog, author) VALUES ($1, $2, $3, $4, $5, $6) RETURNING post_id";
        try {
            std::string content = "Creating Post !<br>";
            int postId;
            auto result = clientPtr->execSqlSync(query, url, title, subtitle, content, isBlog, author);
            content.append("done .... <br>Adding Tags<br>");
            LOG_TRACE << "Url: " << url << ", Title: " << title << ", Subtitle: " << subtitle << ", Author: " << author << ", Content: " << content;
            for (auto row : result) {
                postId = row["post_id"].as<int>();
            }
            std::map<std::string, int> tagMap;
            query = "SELECT tag_id, tag FROM dwTags WHERE tag in ($1)";
            result = clientPtr->execSqlSync(query, tags);
            for (auto row : result) {
                tagMap[row["tag"].as<std::string>()] = row["tag_id"].as<int>();
            }
            std::vector<std::string> newTags;
            for (int i = 0; i < keywords.size(); i++) {
                if (tagMap.count(keywords[i]) == 0) {
                    newTags.push_back(keywords[i]);
                }
            }
            for (int i = 0; i < newTags.size(); i++) {
                query = "INSERT INTO dwTags (tag) VALUES ($1) RETURNING tag_id";
                auto result = clientPtr->execSqlSync(query, newTags[i]);
                for (auto row : result) {
                    tagMap[newTags[i]] = row["tag_id"].as<int>();
                }
            }
            content.append("done<br>Assigning Tags....<br>");
            for (int i = 0; i < tagMap.size(); i++) {
                query = "INSERT INTO dwTagsAssigned (tag_id, post_id) VALUES ($1, $2)";
                result = clientPtr->execSqlSync(query, tags);
            }
            content.append("done");
            auto site = new website(keywords, "en", "Creating Post !", content);
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

void Blog::renderPost(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string url) {
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT dwBlog.title, dwBlog.subtitle, dwBlog.content, dwUsers.name, dwBlog.create_timestamp, dwBlog.tags FROM dwBlog, dwUsers WHERE dwBlog.author = dwUsers.id AND isBlog=1 AND url=$1 ORDER BY create_timestamp DESC LIMIT 1";
    auto result = clientPtr->execSqlSync(query, url);
    std::vector<std::string> keywords;
    for (auto row : result) {
        std::string content = website::getPost(row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["create_timestamp"].as<std::string>(), row["tags"].as<std::string>());
        auto site = new website(keywords, "en", row["title"].as<std::string>(), row["content"].as<std::string>());          // TODO: need to save/get language from database
        callback(site->getPage());                                                                                          // TODO: need to implement keywords/tags
        return;
    }
}

void Blog::renderCategory(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string category) {
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT dwBlog.title, dwBlog.subtitle, dwBlog.content, dwUsers.name, dwBlog.create_timestamp, dwBlog.tags FROM dwBlog, dwUsers WHERE dwBlog.author = dwUsers.id AND isBlog=1 AND url=$1 ORDER BY create_timestamp DESC LIMIT 1";
    auto result = clientPtr->execSqlSync(query, category);
    std::vector<std::string> keywords;
    for (auto row : result) {
        std::string content = website::getPost(row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["create_timestamp"].as<std::string>(), row["tags"].as<std::string>());
        auto site = new website(keywords, "en", row["title"].as<std::string>(), row["content"].as<std::string>());
        callback(site->getPage());
        return;
    }
}
