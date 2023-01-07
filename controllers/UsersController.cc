#include <trantor/utils/Date.h>
#include "UsersController.h"
#include "Website.h"

void UsersController::login(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) {
    if (req->getMethod() == drogon::HttpMethod::Post) {
        auto params = req->getParameters();
        std::string token;
        for (auto param : params) {
            std::string key = std::get<0>(param);
            std::string value = std::get<1>(param);
            if (key == "password") {
                token = value;
            }
        }
        auto email = req->session()->get<std::string>("email");
        std::string query = "SELECT * FROM dwUsers WHERE email=?;";
        auto clientPtr = drogon::app().getDbClient("dwebsite");
        auto result = clientPtr->execSqlSync(query, email);
        if (result.size() > 0) {
            for (auto row : result) {
                auto token = req->session()->get<std::string>("loginToken");
                std::string password = row["password"].as<std::string>();
                if (password == token) {
                    req->session()->insert("loginTimeout", trantor::Date::date().microSecondsSinceEpoch() + (31 * 24 * 60 * 60));
                }
            }
        }
    } else if (req->getMethod() == drogon::HttpMethod::Get) {
        std::string form = "<form action='' method='post'><label for='username'>Username:</label><input type='text' name='username' required><br><label for='password'>Password:</label><input type='password' name='password'><br><input type='submit' value='submit'><form>";
        std::vector<std::string> keywords;
        auto site = new website(keywords, "en", "Login", form);
        callback(site->getPage());
        return;
    }
}