#pragma once

#include <string>
#include <drogon/HttpController.h>
#include <trantor/utils/Utilities.h>


class User : public drogon::HttpController<User>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(User::login, "/login", drogon::Get, drogon::Post);
    METHOD_LIST_END

    std::string getPublicKey(std::string);
    bool isAuthorized(std::string);
    void login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    bool verifySignature(std::vector<std::string>);
};
