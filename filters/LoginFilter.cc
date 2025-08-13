#include <string>
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include <trantor/utils/Date.h>
#include "LoginFilter.h"
#include "helpers/helpers.h"

void LoginFilter::doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&fccb)
{
    std::string timeout = req->session()->get<std::string>("exp");
    std::string email = req->session()->get<std::string>("email");
    long now = trantor::Date::date().microSecondsSinceEpoch();
    long expiration = 0;
    if (!timeout.empty()) {
        expiration = std::stol(timeout, nullptr, 10);
    }
    LOG_DEBUG << "now: " << now << ", timeout: " << expiration;
    if (expiration > now) {
        std::string query = "SELECT token FROM " + helpers::TablePrefix + "Users WHERE email = '$1'";
        auto clientPtr = drogon::app().getDbClient();
        auto result = clientPtr->execSqlSync(query, email);
        std::string token;
        for (auto row : result) {
            token = row["token"].as<std::string>();
        }
        if (token == timeout) {
            fccb();
            return;
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    fcb(res);
}
