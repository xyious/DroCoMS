#pragma once

#include <drogon/HttpController.h>


class User : public drogon::HttpController<User>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(User::login, "/login", drogon::Get, drogon::Post);
    METHOD_LIST_END

    void login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
};
