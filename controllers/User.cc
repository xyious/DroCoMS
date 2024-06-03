#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <trantor/utils/Date.h>
#include <trantor/utils/Utilities.h>
#include "helpers/helpers.h"
#include "User.h"
#include "Website.h"

void User::login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    auto params = req->getParameters();
    std::string credential, token;
    for (auto param : params) {
        std::string key = std::get<0>(param);
        std::string value = std::get<1>(param);
        if (key == "credential") {
            credential = value;
        } else if (key == "g_csrf_token") {
            token = value;
        }
        LOG_TRACE << key << ", " << value;
    }
    if (token == "540103bc95d67db7" && credential.length() > 0) {
        std::vector<std::string> parts = helpers::split(credential, ".");
        for (auto part : parts) {
            std::string decodedPart = drogon::utils::base64Decode(part);
            LOG_TRACE << decodedPart;
            std::size_t startPos = decodedPart.find("\"email\":\"");
            if (startPos != std::string::npos) {
                std::string email = decodedPart.substr(startPos + 7, decodedPart.find("\"", startPos));
                LOG_TRACE << email;
                if (isAuthorized(email)) {
                    req->session()->insert("loginTimeout", duration);
                }
            }
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    callback(res);
}

bool isAuthorized(std::string email) {
    std::string query = "SELECT username FROM Users WHERE email = $1";
}
