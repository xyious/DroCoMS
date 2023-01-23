#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <trantor/utils/Date.h>
#include "../helpers/base64.h"
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
        std::string query = "UPDATE dwUsers SET token=$1 WHERE email=$2;";
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        std::string password = email + std::to_string(trantor::Date::date().microSecondsSinceEpoch());
        unsigned char hash[SHA512_DIGEST_LENGTH];
        EVP_Digest(password.c_str(),password.size(),hash,NULL,EVP_sha512(),NULL);
        std::vector<std::uint8_t> digest;
        for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
            digest.push_back(hash[i]);
        }
        token = base64::encode(digest);
        auto result = clientPtr->execSqlSync(query, token, email);
        req->session()->insert("email", email);
        auto site = new website(keywords, "en", "Login", "Sending Login Email");
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
                        LOG_TRACE << "Modify loginTimeout, duration: " << duration;
                        req->session()->modify<long>("loginTimeout", [duration](long expiration) {expiration = duration;});
                    } else {
                        LOG_TRACE << "Insert loginTimeout, duration: " << duration;
                        req->session()->insert("loginTimeout", duration);
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
    return;
}