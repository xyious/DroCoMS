#include <regex>
#include <map>
#include <iostream>
#include <fstream>
#include "helpers/helpers.h"
#include "Website.h"
#include "Blog.h"

void Blog::create(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    std::string title, subtitle, tags, content, author, language, isBlog = "0";
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
            } else if (key == "language") {
                language = value;
            } else if (key == "author") {
                author = value;
            } else if (key == "isBlog") {
                if (value == "on") {
                    isBlog = "1";
                }
            }
        }
        keywords = helpers::split(tags, ",");
        for (auto tag : keywords) {
            tag = std::regex_replace(tag, std::regex(" "), "-");
        }
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        std::string url = std::regex_replace(title, std::regex(" "), "-");
        url = std::regex_replace(url, std::regex("[^A-Za-z0-9-_]"), "");
        LOG_TRACE << "Url: " << url << ", Title: " << title << ", Subtitle: " << subtitle << ", Author: " << author << ", Content: " << content;
        std::string query = "INSERT INTO dwblog (url, title, subtitle, content, isBlog, author, language) VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING post_id";
        try {
            std::string log = "Creating Post !<br>";
            int postId;
            auto result = clientPtr->execSqlSync(query, url, title, subtitle, content, isBlog, author, language);
            log.append("done .... <br>Adding Tags<br>");
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
            log.append("done<br>Assigning Tags....<br>");
            for (const auto& [key, value] : tagMap) {
                query = "INSERT INTO dwTagsAssigned (tag_id, post_id) VALUES ($1, $2)";
                result = clientPtr->execSqlSync(query, value, postId);
            }
            log.append("done");
            auto site = new website(keywords, "en", "Creating Post !", log, getLeftSidebar(), getRightSidebar());
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
    form.append("'><br><label for='language'>Language:</label><select name='language'><option value='en-US'>English</option><option value='de-DE'>German</option></select><label for='author'>Author:</label><select name='author'>");
    for (auto row : result) {
        form.append("<option value='");
        form.append(row["id"].as<std::string>());
        form.append("'>");
        form.append(row["name"].as<std::string>());
        form.append("</option>");
    }
    form.append("</select><br><label for='isBlog'>isBlog ?:</label><input type='checkbox' id='isBlog' name='isBlog' checked><br><label for='tags'>Tags:</label><input type='text' name='tags' value='");
    form.append(tags);
    form.append("'><br><label for='content'>Content:</label><textarea name='content' cols='128' rows='50'>");
    form.append(content);
    form.append("</textarea><br><input type='submit' value='submit'><form>");
    auto site = new website(keywords, "en", "Create Post", form, getLeftSidebar(), getRightSidebar());
    callback(site->getPage());
    createSitemap();
    return;
}

void Blog::renderPost(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string url) {
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT dwBlog.language, dwBlog.title, dwBlog.subtitle, dwBlog.url, dwBlog.content, dwUsers.name, dwBlog.create_timestamp FROM dwBlog, dwUsers WHERE dwBlog.author = dwUsers.id AND isBlog=1 AND url=$1 ORDER BY create_timestamp DESC LIMIT 1";
    auto result = clientPtr->execSqlSync(query, url);
    std::vector<std::string> keywords;
    for (auto row : result) {
        std::string content = website::getPost(row["url"].as<std::string>(), row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["create_timestamp"].as<std::string>(), keywords);
        auto site = new website(keywords, row["language"].as<std::string>(), row["title"].as<std::string>(), content, getLeftSidebar(), getRightSidebar());
        callback(site->getPage());
        return;
    }
}

void Blog::renderCategory(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string category) {
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT dwBlog.language, dwBlog.title, dwBlog.subtitle, dwBlog.url, dwBlog.content, dwUsers.name, dwBlog.create_timestamp FROM dwBlog, dwUsers, dwTags, dwTagsAssigned WHERE dwBlog.author = dwUsers.id AND isBlog=1 AND dwBlog.post_id = dwTags.post_id AND dwTags.tag_id = dwTagsAssigned.tag_id AND dwTags.tag = $1 ORDER BY create_timestamp DESC LIMIT 3";
    auto result = clientPtr->execSqlSync(query, category);
    std::vector<std::string> keywords;
    std::string content, language, title;
    for (auto row : result) {
        content.append(website::getPost(row["url"].as<std::string>(), row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["create_timestamp"].as<std::string>(), keywords));
        if (title.empty()) {
            title = row["title"].as<std::string>();
        }
        if (language.empty()) {
            language = row["language"].as<std::string>();
        }
    }
    auto site = new website(keywords, language, title, content, getLeftSidebar(), getRightSidebar());
    callback(site->getPage());
    return;
}

void Blog::renderArchive(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT post_id, title, url FROM dwBlog ORDER BY create_timestamp DESC";
    auto result = clientPtr->execSqlSync(query);
    std::vector<std::string> keywords;
    std::map<int, std::string> links;
    std::string content = "<table id='Archive'><tr><th>Title</th><th>Keywords</th></tr>";
    for (auto row : result) {
        std::string link = "<tr><td><a href='";
        link.append(row["url"].as<std::string>()).append("'>").append(row["title"].as<std::string>()).append("</a></td><td>");
        links[row["post_id"].as<int>()] = link;
    }
    for (auto link : links) {
        query = "SELECT dwtags.tag FROM dwTags, dwTagsAssigned WHERE dwTags.tag_id = dwTagsAssigned.tag_id AND dwTagsAssigned.post_id = $1";
        auto result = clientPtr->execSqlSync(query, link.first);
        for (auto row : result) {
            std::string tag = row["tag"].as<std::string>();
            keywords.push_back(tag);
            link.second.append("<a href='https://xyious.com/Categories/").append(tag).append("'>").append(tag).append("</a> ");
        }
        content.append(link.second).append("</td></tr>");
    }
    content.append("</table>");
    content = "<div class='post-container'><h1>Archive</h1><div class='post-content-container'>" + content + "</div>";
    auto site = new website(keywords, "", "Blog Archive", content, getLeftSidebar(), getRightSidebar());
    callback(site->getPage());
}

void Blog::renderHome(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT dwBlog.url, dwBlog.title, dwBlog.subtitle, dwBlog.content, dwUsers.name, dwBlog.create_timestamp FROM dwBlog, dwUsers WHERE dwBlog.author = dwUsers.id AND isBlog=1 ORDER BY create_timestamp DESC LIMIT 3";
    auto result = clientPtr->execSqlSync(query);
    std::string content, title;
    for (auto row : result) {
        if (title.empty()) {
            title = row["title"].as<std::string>();
        }
        content.append(website::getPost(row["url"].as<std::string>(), row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["create_timestamp"].as<std::string>(), keywords));
    }
    auto site = new website(keywords, "en-US", title, content, getLeftSidebar(), getRightSidebar());
    callback(site->getPage());
}

std::string Blog::getLeftSidebar() {
    std::string result = "<div class='left-sidebar'><ul>";
    result.append("<li><h1><a href='https://xyious.com'>Home</a></h1></li>");
    result.append("<li><h1><a href='https://xyious.com/Blog/Archive'>Archive</a></h1></li>");
    result.append("<li><h1><a href='https://spicylesbians.etsy.com'>My Etsy Shop</a></h1></li>")
    result.append("</ul></div>");
    return result;
}

std::string Blog::getRightSidebar() {
    std::string result = "<div class='right-sidebar'>";
    result.append("</div>");
    return result;
}

void Blog::createSitemap() {
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    std::string query = "SELECT url FROM dwBlog";
    clientPtr->execSqlAsync(query, [](const drogon::orm::Result &result) {
        std::ofstream sitemap;
        sitemap.open("sitemap.txt", std::ios::out | std::ios::trunc);
        for (auto row : result) {
            sitemap << "http://xyious.com/" << row["url"].as<std::string>() << std::endl;
        }
        sitemap.close();
    },
    [](const drogon::orm::DrogonDbException &e) {
        LOG_ERROR << "Exception in blog.cc:174: " << e.base().what();
    });
    return;
}