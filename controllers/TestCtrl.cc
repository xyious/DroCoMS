#include <vector>
#include <string>
#include "TestCtrl.h"
#include "Website.h"

void TestCtrl::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    auto resp=HttpResponse::newHttpResponse();
    //NOTE: The enum constant below is named "k200OK" (as in 200 OK), not "k2000K".
    resp->setStatusCode(k200OK);
    resp->setContentTypeCode(CT_TEXT_HTML);
    std::vector<std::string> keywords;
    auto site = new website(keywords, "en", "uhhh .... hi ?", "Hello World !");
    resp->setBody(site->getPage());
    callback(resp);
}
