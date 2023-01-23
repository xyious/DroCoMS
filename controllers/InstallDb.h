#pragma once

#include <drogon/HttpSimpleController.h>

class InstallDb : public drogon::HttpSimpleController<InstallDb>
{
  public:
    void asyncHandleHttpRequest(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    PATH_ADD("/InstallDb", drogon::Get, drogon::Post);
    PATH_LIST_END
};
