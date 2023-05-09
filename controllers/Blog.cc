#include <regex>
#include <map>
#include <iostream>
#include <fstream>
#include "helpers/Constants.h"
#include "helpers/helpers.h"
#include "Website.h"
#include "Blog.h"

void Blog::create(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    std::string title, subtitle, tags, content, author, language, isBlog = "0", category;
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
            } else if (key == "category") {
                category = value;
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
        auto clientPtr = drogon::app().getDbClient();
        std::string url = std::regex_replace(title, std::regex(" "), "-");
        url = std::regex_replace(url, std::regex("[^A-Za-z0-9-_]"), "");
        LOG_TRACE << "Url: " << url << ", Title: " << title << ", Subtitle: " << subtitle << ", Author: " << author << ", Content: " << content << ", Category: " << category;
        std::string query = "INSERT INTO " + helpers::TablePrefix + "blog (url, title, subtitle, content, isBlog, author, language, category) VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING post_id";
        try {
            std::string log = "Creating Post !<br>";
            int postId;
            auto result = clientPtr->execSqlSync(query, url, title, subtitle, content, isBlog, author, language, category);
            log.append("done .... <br>Adding Tags<br>");
            for (auto row : result) {
                postId = row["post_id"].as<int>();
            }
            std::map<std::string, int> tagMap;
            query = "SELECT tag_id, tag FROM " + helpers::TablePrefix + "Tags WHERE tag in ($1)";
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
                query = "INSERT INTO " + helpers::TablePrefix + "Tags (tag) VALUES ($1) RETURNING tag_id";
                auto result = clientPtr->execSqlSync(query, newTags[i]);
                for (auto row : result) {
                    tagMap[newTags[i]] = row["tag_id"].as<int>();
                }
            }
            log.append("done<br>Assigning Tags....<br>");
            for (const auto& [key, value] : tagMap) {
                query = "INSERT INTO " + helpers::TablePrefix + "TagsAssigned (tag_id, post_id) VALUES ($1, $2)";
                result = clientPtr->execSqlSync(query, value, postId);
            }
            log.append("done");
            auto site = new website(keywords, "en", "Creating Post !", log, getLeftSidebar(), getRightSidebar(keywords));
            callback(site->getPage());
            return;
        }
        catch (std::exception const& e) {
            LOG_ERROR << "Exception in Blog.cc: " << e.what();
        }
    }
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT id, name FROM " + helpers::TablePrefix + "categories";
    auto result = clientPtr->execSqlSync(query);
    std::string form = "<form action='' method='post'><label for='title'>Title:</label><input type='text' name='title' value='" + title +"' required><br><label for='subtitle'>Subtitle:</label><input type='text' name='subtitle' value='" + subtitle + "'><br><label for='category'>Category:</label><select name='category'>";
    for (auto row : result) {
        form.append("<option value='");
        form.append(row["id"].as<std::string>());
        form.append("'>");
        form.append(row["name"].as<std::string>());
        form.append("</option>");
    }
    form.append("</select><label for='language'>Language:</label><select name='language'><option value='en-US'>English</option><option value='de-DE'>German</option></select><label for='author'>Author:</label><select name='author'>");
    query = "SELECT id, name FROM " + helpers::TablePrefix + "users";
    result = clientPtr->execSqlSync(query);
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
    auto site = new website(keywords, "en-US", "Create Post", form, getLeftSidebar(), getRightSidebar(keywords));
    callback(site->getPage());
    createSitemap();
    return;
}

void Blog::createCategory(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    std::string name, description, parent, language, isBlog = "0", isExternal = "0";
    if (req->getMethod() == drogon::HttpMethod::Post) {
        auto params = req->getParameters();
        for (auto param : params) {
            std::string key = std::get<0>(param);
            std::string value = std::get<1>(param);
            LOG_TRACE << "key: " << key << ", value: " << value;
            if (key == "name") {
                name = value;
            } else if (key == "description") {
                description = value;
            } else if (key == "parent") {
                parent = value;
            } else if (key == "language") {
                language = value;
            } else if (key == "isBlog") {
                if (value == "on") {
                    isBlog = "1";
                }
            } else if (key == "isExternal") {
                if (value == "on") {
                    isBlog = "1";
                }
            }
        }
        auto clientPtr = drogon::app().getDbClient();
        name = std::regex_replace(name, std::regex(" "), "-");
        name = std::regex_replace(name, std::regex("[^A-Za-z0-9-_]"), "");
        LOG_TRACE << "Name: " << name << ", Description: " << description << ", Parent: " << parent << ", isBlog: " << isBlog;
        std::string query = "INSERT INTO " + helpers::TablePrefix + "categories (name, description, parent, language, isBlog, isExternal) VALUES ($1, $2, $3, $4, $5, $6)";
        try {
            std::string log = "Creating Category !<br>";
            auto result = clientPtr->execSqlSync(query, name, description, parent, language, isBlog, isExternal);
            log.append("done .... ");
            auto site = new website(keywords, "en", "Creating Category !", log, getLeftSidebar(), getRightSidebar(keywords));
            callback(site->getPage());
            return;
        }
        catch (std::exception const& e) {
            LOG_ERROR << "Exception in Blog.cc: " << e.what();
        }
    }
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT id, name FROM " + helpers::TablePrefix + "categories";
    auto result = clientPtr->execSqlSync(query);
    std::string form = "<form action='' method='post'><label for='name'>Name:</label><input type='text' name='name' value='";
    form.append(name);
    form.append("' required><br><label for='description'>Description:</label><input type='text' name='description' value='");
    form.append(description);
    form.append("'><br><label for='language'>Language:</label><select name='language'><option value='en-US'>English</option><option value='de-DE'>German</option></select><label for='parent'>Parent:</label><select name='parent'>");
    for (auto row : result) {
        form.append("<option value='");
        form.append(row["id"].as<std::string>());
        form.append("'>");
        form.append(row["name"].as<std::string>());
        form.append("</option>");
    }
    form.append("</select><br><label for='isBlog'>isBlog ?:</label><input type='checkbox' id='isBlog' name='isBlog' checked><br><label for='isExternal'>isExternal ?:</label><input type='checkbox' id='isExternal' name='isExternal'><br><input type='submit' value='submit'><form>");
    auto site = new website(keywords, "en-US", "Create Category", form, getLeftSidebar(), getRightSidebar(keywords));
    createSitemap();
    callback(site->getPage());
}

void Blog::renderPost(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string url) {
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT " + helpers::TablePrefix + "Blog.post_id, " + helpers::TablePrefix + "Blog.language, " + helpers::TablePrefix + "Blog.title, " + helpers::TablePrefix + "Blog.subtitle, " + helpers::TablePrefix + "Blog.url, " + helpers::TablePrefix + "Blog.content, " + helpers::TablePrefix + "Users.name, to_char(" +  helpers::TablePrefix + "Blog.create_timestamp,'Month DD YYYY') as timestamp FROM " + helpers::TablePrefix + "Blog, " + helpers::TablePrefix + "Users WHERE " + helpers::TablePrefix + "Blog.author = " + helpers::TablePrefix + "Users.id AND isBlog=1 AND url=$1 ORDER BY " + helpers::TablePrefix + "Blog.create_timestamp DESC LIMIT 1";
    auto result = clientPtr->execSqlSync(query, url);
    std::vector<std::string> keywords;
    unsigned int id;
    std::string language, title, content;
    for (auto row : result) {
        id = row["post_id"].as<int>();
        language = row["language"].as<std::string>();
        title = row["title"].as<std::string>();
        content = website::getPost(row["url"].as<std::string>(), row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["timestamp"].as<std::string>(), keywords);
    }
    query = "SELECT " + helpers::TablePrefix + "Tags.tag_id, " + helpers::TablePrefix + "Tags.tag FROM " + helpers::TablePrefix + "Tags, " + helpers::TablePrefix + "TagsAssigned WHERE " + helpers::TablePrefix + "Tags.tag_id = " + helpers::TablePrefix + "TagsAssigned.tag_id AND " + helpers::TablePrefix + "TagsAssigned.post_id = $1";
    result = clientPtr->execSqlSync(query, id);
    for (auto row : result) {
        keywords.push_back(row["tag"].as<std::string>());
    }
    auto site = new website(keywords, language, title, content, getLeftSidebar(), getRightSidebar(keywords));
    callback(site->getPage());
}

void Blog::renderTag(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string tag) {
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT " + helpers::TablePrefix + "Blog.language, " + helpers::TablePrefix + "Blog.title, " + helpers::TablePrefix + "Blog.subtitle, " + helpers::TablePrefix + "Blog.url, " + helpers::TablePrefix + "Blog.content, " + helpers::TablePrefix + "Users.name, to_char(" +  helpers::TablePrefix + "Blog.create_timestamp,'Month DD YYYY') as timestamp FROM " + helpers::TablePrefix + "Blog, " + helpers::TablePrefix + "Users, " + helpers::TablePrefix + "Tags, " + helpers::TablePrefix + "TagsAssigned WHERE " + helpers::TablePrefix + "Blog.author = " + helpers::TablePrefix + "Users.id AND isBlog=1 AND " + helpers::TablePrefix + "Blog.post_id = " + helpers::TablePrefix + "TagsAssigned.post_id AND " + helpers::TablePrefix + "Tags.tag_id = " + helpers::TablePrefix + "TagsAssigned.tag_id AND " + helpers::TablePrefix + "Tags.tag = $1 ORDER BY " + helpers::TablePrefix + "blog.create_timestamp DESC LIMIT 3";
    auto result = clientPtr->execSqlSync(query, tag);
    std::vector<std::string> keywords;
    std::string content, language, title;
    for (auto row : result) {
        content.append(website::getPost(row["url"].as<std::string>(), row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["timestamp"].as<std::string>(), keywords));
        if (title.empty()) {
            title = row["title"].as<std::string>();
        }
        if (language.empty()) {
            language = row["language"].as<std::string>();
        }
    }
    auto site = new website(keywords, language, title, content, getLeftSidebar(), getRightSidebar(keywords));
    callback(site->getPage());
}

void Blog::renderArchive(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT post_id, title, url FROM " + helpers::TablePrefix + "Blog ORDER BY " + helpers::TablePrefix + "blog.create_timestamp DESC";
    auto result = clientPtr->execSqlSync(query);
    std::vector<std::string> keywords;
    std::map<int, std::string> links;
    std::string content = "<table id='Archive'><tr><th>Title</th><th>Keywords</th></tr>";
    for (auto row : result) {
        std::string link = "<tr><td><a href='" + helpers::BaseURL + "/Blog/";
        link.append(row["url"].as<std::string>() + "'>" + row["title"].as<std::string>() + "</a></td><td>");
        links[row["post_id"].as<int>()] = link;
    }
    for (auto link : links) {
        query = "SELECT " + helpers::TablePrefix + "tags.tag FROM " + helpers::TablePrefix + "Tags, " + helpers::TablePrefix + "TagsAssigned WHERE " + helpers::TablePrefix + "Tags.tag_id = " + helpers::TablePrefix + "TagsAssigned.tag_id AND " + helpers::TablePrefix + "TagsAssigned.post_id = $1";
        auto result = clientPtr->execSqlSync(query, link.first);
        for (auto row : result) {
            std::string tag = row["tag"].as<std::string>();
            keywords.push_back(tag);
            link.second.append("<a href='" + helpers::BaseURL + "/Tags/" + tag + "'>" + tag + "</a> ");
        }
        content.append(link.second + "</td></tr>");
    }
    content.append("</table>");
    content = "<div class='post-container'><h1>Archive</h1><div class='post-content-container'>" + content + "</div>";
    auto site = new website(keywords, "en-US", "Blog Archive", content, getLeftSidebar(), getRightSidebar(keywords));
    callback(site->getPage());
}

void Blog::renderHome(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT " + helpers::TablePrefix + "Blog.post_id, " + helpers::TablePrefix + "Blog.url, " + helpers::TablePrefix + "Blog.title, " + helpers::TablePrefix + "Blog.subtitle, " + helpers::TablePrefix + "Blog.content, " + helpers::TablePrefix + "Users.name, to_char(" +  helpers::TablePrefix + "Blog.create_timestamp,'Month DD YYYY') as timestamp FROM " + helpers::TablePrefix + "Blog, " + helpers::TablePrefix + "Users WHERE " + helpers::TablePrefix + "Blog.author = " + helpers::TablePrefix + "Users.id AND isBlog=1 ORDER BY " + helpers::TablePrefix + "Blog.create_timestamp DESC LIMIT 3";
    auto result = clientPtr->execSqlSync(query);
    std::string content, title;
    std::vector<unsigned int> ids, tags;
    for (auto row : result) {
        if (title.empty()) {
            title = row["title"].as<std::string>();
        }
        content.append(website::getPost(row["url"].as<std::string>(), row["title"].as<std::string>(), row["subtitle"].as<std::string>(), row["content"].as<std::string>(), row["name"].as<std::string>(), row["timestamp"].as<std::string>(), keywords));
        ids.push_back(row["post_id"].as<unsigned int>());
    }
    if (ids.size() > 0) {
        query = "SELECT " + helpers::TablePrefix + "Tags.tag FROM " + helpers::TablePrefix + "Tags, " + helpers::TablePrefix + "TagsAssigned WHERE " + helpers::TablePrefix + "Tags.tag_id = " + helpers::TablePrefix + "TagsAssigned.tag_id AND " + helpers::TablePrefix + "TagsAssigned.post_id IN ($1)";
        result = clientPtr->execSqlSync(query, ids.front());
        for (auto row : result) {
            keywords.push_back(row["tag"].as<std::string>());
        }
    }
    auto site = new website(keywords, "en-US", title, content, getLeftSidebar(), getRightSidebar(keywords));
    callback(site->getPage());
}

std::string Blog::getLeftSidebar() {
    std::string result = "<li><h3><a href='" + helpers::BaseURL + "/Blog/Archive'>Archive</a></h3></li>";
    return result;
}

std::string Blog::getRightSidebar(std::vector<std::string> keywords) {
    std::string result = "";
    for (auto tag : keywords) {
        result.append("<li><a href='" + helpers::BaseURL + "/Tags/" + tag + "'>" + tag + "</a></ul>");
    }
    return result;
}

void Blog::createSitemap() {
    auto clientPtr = drogon::app().getDbClient();
    std::string query = "SELECT url FROM " + helpers::TablePrefix + "blog";
    clientPtr->execSqlAsync(query, [](const drogon::orm::Result &result) {
        std::ofstream sitemap;
        sitemap.open("sitemap.txt", std::ios::out | std::ios::trunc);
        for (auto row : result) {
            sitemap << helpers::BaseURL << "/" << row["url"].as<std::string>() << std::endl;
        }
        sitemap.close();
    },
    [](const drogon::orm::DrogonDbException &e) {
        LOG_ERROR << "Exception in blog.cc: " << e.base().what();
    });
}
