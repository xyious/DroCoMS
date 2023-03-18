#pragma once

#include <drogon/HttpController.h>

class Blog : public drogon::HttpController<Blog>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(Blog::create, "/Blog/Create", drogon::Get, drogon::Post, "LoginFilter");
    ADD_METHOD_TO(Blog::renderPost, "/Blog/{url}", drogon::Get);
    ADD_METHOD_TO(Blog::renderCategory, "/Category/{category}", drogon::Get);
    ADD_METHOD_TO(Blog::renderArchive, "/Blog/Archive", drogon::Get);
    ADD_METHOD_TO(Blog::renderHome, "/", drogon::Get);
    METHOD_LIST_END
    
    void create(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void renderArchive(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void renderHome(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void renderPost(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string url);
    void renderCategory(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string category);
    std::string getLeftSidebar();
    std::string getRightSidebar();
    void createSitemap();
};
