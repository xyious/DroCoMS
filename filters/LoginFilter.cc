#include <trantor/utils/Date.h>
#include "LoginFilter.h"

void LoginFilter::doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&fccb)
{
    std::string timeout = req->session()->get<std::string>("exp");
    long now = trantor::Date::date().microSecondsSinceEpoch();
    long expiration = 0;
    if (!timeout.empty()) {
        expiration = std::stol(timeout, nullptr, 10);
    }
    LOG_DEBUG << "now: " << now << ", timeout: " << expiration;
    if (expiration > now) {
        fccb();
        return;
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    fcb(res);
}
