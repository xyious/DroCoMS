#include <vector>
#include <string>
#include <exception>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <drogon/orm/DbClient.h>
#include <drogon/HttpTypes.h>
#include "InstallDb.h"
#include "../helpers/base64.h"
#include "Website.h"

void InstallDb::asyncHandleHttpRequest(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    std::string query = "SELECT * FROM dwUsers;";
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    bool dbExists = false;
    try {
        auto result = clientPtr->execSqlSync(query);
        if (result.size() > 0) {
            dbExists = true;
        }
    } catch (std::exception& exc) {
		LOG_ERROR << exc.what();
    }
    std::string output = "";
    if (dbExists) {
        auto site = new website(keywords, "en", "Create Database", "Database already exists !", "", "");
        callback(site->getPage());
        return;
    } else {
        if (req->getMethod() == drogon::HttpMethod::Post) {
            std::string username, email, name, token;
            auto params = req->getParameters();
            for (auto param : params) {
                std::string key = std::get<0>(param);
                std::string value = std::get<1>(param);
                if (key == "name") {
                    name = value;
                } else if (key == "username") {
                    username = value;
                } else if (key == "email") {
                    email = value;
                }
                LOG_TRACE << "param " << key << ", value: " << value;
            }
            query = "CREATE TABLE IF NOT EXISTS dwBlog (post_id SERIAL PRIMARY KEY, url VARCHAR(255), title VARCHAR(255), subtitle VARCHAR(255), language CHAR(5), content text, author int, isBlog int, create_timestamp timestamp DEFAULT current_timestamp, edit_timestamp timestamp);";
            output = "Installing Database....<br>";
            output.append("Creating blog table....<br>");
            auto result = clientPtr->execSqlSync(query);
            output.append(".... done<br>Creating Tags table....<br>");
            query = "CREATE TABLE IF NOT EXISTS dwTags (tag_id SERIAL PRIMARY KEY, tag VARCHAR(255), description VARCHAR(255));";
            result = clientPtr->execSqlSync(query);
            output.append(".... done<br>Creating AssignedTags table....<br>");
            query = "CREATE TABLE IF NOT EXISTS dwTagsAssigned (tag_id INT, post_id INT);";
            result = clientPtr->execSqlSync(query);
            output.append(".... done<br>Creating users table....<br>");
            query = "CREATE TABLE IF NOT EXISTS dwUsers (id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE, email VARCHAR(255), name VARCHAR(255), token VARCHAR(100), expiration BIGINT, can_post INT, create_timestamp timestamp DEFAULT current_timestamp);";
            result = clientPtr->execSqlSync(query);
            output.append(".... done<br>Creating user....<br>");
            query = "INSERT INTO dwUsers (username, email, name, token, expiration, can_post) VALUES ($1, $2, $3, $4, $5, $6)";
            std::string password = email + std::to_string(trantor::Date::date().microSecondsSinceEpoch());
            unsigned char hash[SHA512_DIGEST_LENGTH];
            EVP_Digest(password.c_str(),password.size(),hash,NULL,EVP_sha512(),NULL);
            std::vector<std::uint8_t> digest;
            for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
                digest.push_back(hash[i]);
            }
            token = base64::encode(digest);
            unsigned long long expiration = trantor::Date::date().microSecondsSinceEpoch() + (365L * 24 * 60 * 60 * 1000);         // The first user gets to stay logged in for 1 year (because it's me and I make good decisions)
            req->session()->insert("loginTimeout",  expiration);
            result = clientPtr->execSqlSync(query, username, email, name, token, expiration, 1);
            LOG_TRACE << "username: " << username << ", email: " << email << ", name: " << name << ", token: " << token << ", expiration: " << expiration;
            output.append(".... done !!!!");
            auto site = new website(keywords, "en", "Installing Database", output, "", "");
            callback(site->getPage());
            return;
        } else {
            std::string form = "<form action='' method='post'><label for='username'>Username:</label><input type='text' name='username' required><br><label for='email'>Email:</label><input type='email' name='email' required><br><label for='name'>Name:</label><input type='text' name='name'><br><input type='submit' value='submit'><form>";
            auto site = new website(keywords, "en", "Create Database", form, "", "");
            callback(site->getPage());
            return;
        }
    }
}
