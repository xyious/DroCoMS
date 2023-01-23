#pragma once

#include <drogon/HttpController.h>

class Blog : public drogon::HttpController<Blog>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(Blog::create, "/Blog/Create", drogon::Get, drogon::Post, "LoginFilter");
    ADD_METHOD_TO(Blog::renderPost, "/Blog/{url}", drogon::Get);
    METHOD_LIST_END
    
    void create(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void renderPost(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string url);
};
