#include <string>
#include <trantor/utils/Date.h>
#include <trantor/utils/Utilities.h>
#include <json/json.h>
#include <jwt-cpp/jwt.h>
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
        if (parts.length() <= 3) {
            std::string jwt = drogon::utils::base64Decode(part[1]);
            std::string decoded_token = jwt::decode(credential);
            // Look, I have a single user and it's me. I'm not going to make this thread safe or even cache this .... Or get it live.... 
            // for now.... This key /should be/ identical to what's on google's page: https://www.googleapis.com/oauth2/v3/certs
            std::string n = "rHz-FQE9gjFJR_FhnzhBMPpa8NJ2nCfnXLr5LWDJOOaiGqI__Nrm6HHUCpMi52_pLqqVkCihR9xbscZ6UKr9wjp-7YTDN6A9i7QqQAJyNRIMCkJR1z6D95_pam_mIkBVnYjJ_LskOyOHI65Yvuaw6oA9iFlSyucn4B-jZRmp7JyGyU8UMohaOvJB7_boaIoEx_QY8YdoANKrp0WGawEkW6RgopgiHB7D0CXU-c_GDp0TjWCZegQzoV_fDD5eH5mc2Ai3dBylZxgQ-ZxMakYS01nmVr1atkpHT1L9W7PiCP60C8WG1aLIzZTLcABK3BWCmZ3-wBZtHZ0y9kSP35aowQ";
            std::string e = "AQAB";
            try {
                auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::rs256(jwt::helper::decode_base64url(n), jwt::helper::decode_base64url(e)));
                verifier.verify(decoded_token);
                std::string email = decoded_token.get_payload_claim("email").as_string();
                LOG_TRACE << email;
                if (isAuthorized(email)) {
                    req->session()->insert("loginToken", decoded_token);
                }
                LOG_TRACE << "Logged in";
                auto site = website(keywords, "en", "Login", "Logged in....");
                callback(site.getPage());
                return;
            } catch (const std::exception& e) {
                LOG_ERROR << "Error while processing token: " << e.what();
                auto resp = drogon::HttpResponse::newHttpResponse(drogon::k500InternalServerError, drogon::CT_TEXT_PLAIN);
                resp->setBody("Internal server error during token verification.");
                fc(resp);
            }
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    callback(res);
}

