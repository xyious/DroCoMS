#include <trantor/utils/Date.h>
#include "LoginFilter.h"

void LoginFilter::doFilter(const drogon::HttpRequestPtr &req, FilterCallback &&fcb, FilterChainCallback &&fccb)
{
    if (req->session()->find("loginTimeout")) {
        if (req->session()->get<int>("loginTimeout") > trantor::Date::date().microSecondsSinceEpoch()) {
            fccb();
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    fcb(res);
}
