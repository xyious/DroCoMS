#pragma once

#include <drogon/HttpController.h>


class UsersController : public drogon::HttpController<UsersController>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(UsersController::login, "/login", drogon::Get, drogon::Post);
    METHOD_LIST_END

    void login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
};
