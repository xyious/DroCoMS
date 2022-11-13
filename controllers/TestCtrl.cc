#include <vector>
#include <string>
#include "TestCtrl.h"
#include "Website.h"

void TestCtrl::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    auto site = new website(keywords, "en", "uhhh .... hi ?", "Hello World !");
    callback(site->getPage());
}
