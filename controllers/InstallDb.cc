#include <vector>
#include <string>
#include <iostream>
#include <drogon/orm/DbClient.h>
#include "InstallDb.h"
#include "Website.h"

void InstallDb::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    auto resp=HttpResponse::newHttpResponse();
    //NOTE: The enum constant below is named "k200OK" (as in 200 OK), not "k2000K".
    resp->setStatusCode(k200OK);
    resp->setContentTypeCode(CT_TEXT_HTML);
    std::vector<std::string> keywords;
    std::string query = "CREATE TABLE IF NOT EXISTS dwBlog (title VARCHAR(255) PRIMARY KEY, subtitle VARCHAR(255), tags VARCHAR(255), content text, author int, create_timestamp timestamp DEFAULT current_timestamp, edit_timestamp timestamp);";
    std::string output = "Installing Database....<br>";
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    output.append("Creating blog table....<br>");
    auto result = clientPtr->execSqlSync(query);
    output.append(".... done<br>");
    auto site = new website(keywords, "en", "Installing Database", output);
    resp->setBody(site->getPage());
    callback(resp);
}
