#pragma once

#include <drogon/HttpFilter.h>

class LoginFilter : public drogon::HttpFilter<LoginFilter>
{
  public:
    void doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&fccb) override;
};

