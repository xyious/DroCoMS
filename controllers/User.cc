#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <trantor/utils/Date.h>
#include <trantor/utils/Utilities.h>
#include "helpers/helpers.h"
#include "User.h"
#include "Website.h"

bool User::isAuthorized(std::string email) {
    std::string query = "SELECT username FROM " + helpers::TablePrefix + "Users WHERE email = $1";
    auto clientPtr = drogon::app().getDbClient();
    auto result = clientPtr->execSqlSync(query, email);
    for (auto row : result) {
        LOG_TRACE << "User name: " << row["username"].as<std::string>();
        return true;
    }
    return false;
}

void User::login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    auto params = req->getParameters();
    std::string credential;
    bool tokenPresent = false;
    for (auto param : params) {
        std::string key = std::get<0>(param);
        std::string value = std::get<1>(param);
        if (key == "credential") {
            credential = value;
        } else if (key == "g_csrf_token") {
            tokenPresent = true;
        }
        LOG_TRACE << key << ", " << value;
    }
    if (tokenPresent && credential.length() > 0) {
        std::vector<std::string> parts = helpers::split(credential, ".");
        for (auto part : parts) {
            std::string decodedPart = drogon::utils::base64Decode(part);
            LOG_TRACE << decodedPart;
            std::size_t startPos = decodedPart.find("\"email\":\"");
            if (startPos != std::string::npos) {
                startPos += 9;
                std::string email = decodedPart.substr(startPos, decodedPart.find("\"", startPos) - startPos);
                LOG_TRACE << email;
                if (isAuthorized(email)) {
                    req->session()->insert("loginToken", "some gibberish");
                }
                break;
            }
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    callback(res);
}

