#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class Blog : public drogon::HttpSimpleController<Blog>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    PATH_ADD("/Blog/Create",Get, Post);
    PATH_LIST_END
};
