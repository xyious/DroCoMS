#include <trantor/utils/Date.h>
#include "LoginFilter.h"

void LoginFilter::doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&fccb)
{
    long timeout = req->session()->get<long>("loginTimeout");
    long now = trantor::Date::date().microSecondsSinceEpoch();
    LOG_DEBUG << "now: " << now << ", timeout: " << timeout;
    if (timeout > now) {
        fccb();
        return;
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    fcb(res);
}
