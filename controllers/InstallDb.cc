#include <vector>
#include <string>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <drogon/orm/DbClient.h>
#include <drogon/HttpTypes.h>
#include "InstallDb.h"
#include "Website.h"
#include "../helpers/base64.h"

void InstallDb::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    std::vector<std::string> keywords;
    std::string query = "SELECT * FROM dwUsers;";
    auto clientPtr = drogon::app().getDbClient("dwebsite");
    auto result = clientPtr->execSqlSync(query);
    std::string output = "";
    if (result.size() > 0) {
        auto site = new website(keywords, "en", "Create Database", "Database already exists !");
        callback(site->getPage());
        return;
    } else {
        if (req->getMethod() == HttpMethod::Post) {
            std::string username, email, name, password;
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
                } else if (key == "password") {
                    password = value;
                }
            }
            unsigned char hash[SHA512_DIGEST_LENGTH];
            EVP_Digest(password.c_str(),password.size(),hash,NULL,EVP_sha512(),NULL);
            std::vector<std::uint8_t> digest;
            for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
                digest.push_back(hash[i]);
            }
            password = base64::encode(digest);
            query = "CREATE TABLE IF NOT EXISTS dwBlog (title VARCHAR(255) PRIMARY KEY, subtitle VARCHAR(255), tags VARCHAR(255), content text, author int, create_timestamp timestamp DEFAULT current_timestamp, edit_timestamp timestamp);";
            output = "Installing Database....<br>";
            output.append("Creating blog table....<br>");
            auto result = clientPtr->execSqlSync(query);
            output.append(".... done<br>Creating users table....<br>");
            query = "CREATE TABLE IF NOT EXISTS dwUsers (id SERIAL PRIMARY KEY, username VARCHAR(255) UNIQUE, email VARCHAR(255), name VARCHAR(255), password VARCHAR(100), create_timestamp timestamp DEFAULT current_timestamp);";
            result = clientPtr->execSqlSync(query);
            output.append(".... done<br>Creating user....<br>");
            query = "INSERT INTO dwUsers (username, email, name, password) VALUES ($1, $2, $3, $4)";
            result = clientPtr->execSqlSync(query, username, email, name, password);
            output.append(".... done !!!!");
            auto site = new website(keywords, "en", "Installing Database", output);
            callback(site->getPage());
        } else {
            std::string form = "<form action='' method='post'><label for='username'>Username:</label><input type='text' name='username' required><br><label for='email'>Email:</label><input type='email' name='email' required><br><label for='name'>Name:</label><input type='text' name='name'><br><label for='password'>Password:</label><input type='password' name='password'><br><input type='submit' value='submit'><form>";
            auto site = new website(keywords, "en", "Create Database", form);
            callback(site->getPage());
        }
    }
}
