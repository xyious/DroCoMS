#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <trantor/utils/Date.h>
#include <trantor/utils/Utilities.h>
#include "User.h"
#include "Website.h"

void User::login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    std::vector<std::string> keywords;
    auto params = req->getParameters();
    std::string token, email, username;
    for (auto param : params) {
        std::string key = std::get<0>(param);
        std::string value = std::get<1>(param);
        if (key == "email") {
            email = value;
        } else if (key == "username") {
            username = value;
        } else if (key == "password") {
            token = value;
        }
        LOG_TRACE << key << ", " << value;
    }
    if (email.empty() || (!req->session()->get<std::string>("email").empty() && email != req->session()->get<std::string>("email"))) {
        LOG_TRACE << email << " " << req->session()->get<std::string>("email");
        std::string form = "<form action='' method='post'><label for='username'>Username:</label><input type='text' name='username' required><br><label for='email'>Email:</label><input type='email' name='email'><br><input type='submit' value='submit'><form>";
        auto site = new website(keywords, "en", "Login", form);
        callback(site->getPage());
        return;
    }
    if (token.empty()) {
        std::string query = "SELECT * FROM dwUsers WHERE email=$1";
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        auto result = clientPtr->execSqlSync(query, email);
        long duration = 0;
        for (auto row : result) {
            token = row["token"].as<std::string>();
            duration = row["expiration"].as<long>();
        }
        if (duration == 0) {
            std::string query = "UPDATE dwUsers SET token=$1 WHERE email=$2;";
            std::string password = email + std::to_string(trantor::Date::date().microSecondsSinceEpoch());
            unsigned char hash[SHA512_DIGEST_LENGTH];
            EVP_Digest(password.c_str(),password.size(),hash,NULL,EVP_sha512(),NULL);
            token = drogon::utils::base64Encode(hash, SHA512_DIGEST_LENGTH, true);
            auto result = clientPtr->execSqlSync(query, token, email);
            req->session()->insert("email", email);
        }
        auto site = new website(keywords, "en-US", "Login", "Sending Login Email");
        callback(site->getPage());
        return;
    } else {
        std::string query = "SELECT * FROM dwUsers WHERE email=$1;";
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        auto result = clientPtr->execSqlSync(query, email);
        if (result.size() > 0) {
            for (auto row : result) {
                std::string password = row["token"].as<std::string>();
                long duration = row["expiration"].as<long>();
                if (password == token) {
                    if (req->session()->find("loginTimeout")) {
                        LOG_TRACE << "loginTimeout found";
                        req->session()->modify<std::string>("loginTimeout", [duration](std::string expiration) {expiration = std::to_string(duration);});
                    } else {
                        req->session()->insert("loginTimeout", std::to_string(duration));
                    }
                    auto site = new website(keywords, "en", "Login", "Logged in....");
                    callback(site->getPage());
                    return;
                }
            }
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    callback(res);
}