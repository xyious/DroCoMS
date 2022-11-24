#include <vector>
#include <string>
#include "SiteController.h"
#include "Website.h"

void SiteController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    auto site = new website(keywords, "en", "uhhh .... hi ?", "Hello World !");
    callback(site->getPage());
}
