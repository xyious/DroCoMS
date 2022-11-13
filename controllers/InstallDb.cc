#include <vector>
#include <string>
#include <iostream>
#include <drogon/orm/DbClient.h>
#include <drogon/HttpTypes.h>
#include "InstallDb.h"
#include "Website.h"

void InstallDb::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    if (req->getMethod() == HttpMethod::Get) {

    }
    std::vector<std::string> keywords;
    std::string query = "CREATE TABLE IF NOT EXISTS dwBlog (title VARCHAR(255) PRIMARY KEY, subtitle VARCHAR(255), tags VARCHAR(255), content text, author int, create_timestamp timestamp DEFAULT current_timestamp, edit_timestamp timestamp);";
    std::string output = "Installing Database....<br>";
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    output.append("Creating blog table....<br>");
    auto result = clientPtr->execSqlSync(query);
    output.append(".... done<br>Creating users table....<br>");
    query = "CREATE TABLE IF NOT EXISTS dwUsers (id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE, email VARCHAR(255), name VARCHAR(255), password VARCHAR(100), create_timestamp timestamp DEFAULT current_timestamp);";
    result = clientPtr->execSqlSync(query);
    output.append(".... done<br>");
    auto site = new website(keywords, "en", "Installing Database", output);
    callback(site->getPage());
}
