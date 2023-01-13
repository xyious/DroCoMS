#include <string>
#include <trantor/utils/Date.h>
#include "UsersController.h"
#include "Website.h"

void UsersController::login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    if (req->getMethod() == drogon::HttpMethod::Get) {
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
        }
        if (email.empty() || !req->session()->find("email") || req->session()->get<std::string>("email") != email) {
            std::string form = "<form action='' method='post'><label for='username'>Username:</label><input type='text' name='username' required><br><label for='email'>Email:</label><input type='email' name='email'><br><input type='submit' value='submit'><form>";
            std::vector<std::string> keywords;
            auto site = new website(keywords, "en", "Login", form);
            callback(site->getPage());
            return;
        }
        if (token.empty()) {
            std::string query = "UPDATE dwUsers SET password=? WHERE email=?;";
            auto clientPtr = drogon::app().getDbClient("dwebsite");
            password = email + std::to_string(trantor::Date::date().microSecondsSinceEpoch())
            unsigned char hash[SHA512_DIGEST_LENGTH];
            EVP_Digest(password.c_str(),password.size(),hash,NULL,EVP_sha512(),NULL);
            std::vector<std::uint8_t> digest;
            for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
                digest.push_back(hash[i]);
            }
            token = base64::encode(digest);
            auto result = clientPtr->execSqlSync(query, token, email);
            req->session()->insert("email", email);
        } else {
            std::string query = "SELECT * FROM dwUsers WHERE email=?;";
            auto clientPtr = drogon::app().getDbClient("dwebsite");
            auto result = clientPtr->execSqlSync(query, email);
            if (result.size() > 0) {
                for (auto row : result) {
                    auto token = req->session()->get<std::string>("loginToken");
                    std::string password = row["password"].as<std::string>();
                    if (password == token) {
                        req->session()->insert("loginTimeout", trantor::Date::date().microSecondsSinceEpoch() + (31 * 24 * 60 * 60 * 1000));
                    }
                }
            }
        }
    }
}