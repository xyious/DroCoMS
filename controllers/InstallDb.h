#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class InstallDb : public drogon::HttpSimpleController<InstallDb>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    PATH_ADD("/InstallDb",Get);
    PATH_LIST_END
};
