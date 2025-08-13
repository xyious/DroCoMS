#include <string>
#include <drogon/HttpClient.h>
#include <trantor/utils/Date.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/rsa.h>
#include <json/json.h>
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

bool User::verifySignature(std::vector<std::string> data) {
    auto client = drogon::HttpClient::newHttpClient("https://www.googleapis.com");
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/oauth2/v3/certs");
    Json::Value json;
    Json::Reader reader;
    reader.parse(helpers::base64UrlDecode(data[0]), json);
    std::string kid = json["kid"].asString();
    auto resp = client->sendRequest(req);
    if (resp.first != drogon::ReqResult::Ok) {
        LOG_WARN << "Failed to get keys";
        return false;
    }
    std::string body(resp.second->body());
    reader.parse(body, json);
    std::string n_bytes, e_bytes;
    bool verified = false;
    for (const auto &k : json["keys"]) {
        if (!k.isMember("kid")) continue;
        if (kid == k["kid"].asString()) {
            LOG_TRACE << "Found kid: " << kid;
            n_bytes = helpers::base64UrlDecode(k["n"].asString());
            e_bytes = helpers::base64UrlDecode(k["e"].asString());
        }
    }
    auto n_bn = std::unique_ptr<BIGNUM, decltype(&BN_free)>(BN_bin2bn((const unsigned char *)n_bytes.c_str(), n_bytes.size(), nullptr), &BN_free);
    auto e_bn = std::unique_ptr<BIGNUM, decltype(&BN_free)>(BN_bin2bn((const unsigned char *)e_bytes.c_str(), e_bytes.size(), nullptr), &BN_free);
    if (!e_bn || !n_bn) {
        LOG_ERROR << "Failed setting BIGNUMs";
        return false;
    }
    RSA *rsa = RSA_new();
    if (!rsa) {
        LOG_ERROR << "Failed to create RSA";
        return false;
    }
    RSA_set0_key(rsa, n_bn.release(), e_bn.release(), nullptr);
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> public_key(EVP_PKEY_new(), &EVP_PKEY_free);
    if (!EVP_PKEY_assign_RSA(public_key.get(), rsa)) {
        LOG_ERROR << "Failed to assign RSA";
        RSA_free(rsa);
        return false;
    }
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
    if (1 == EVP_DigestVerifyInit(ctx.get(), nullptr, EVP_sha256(), nullptr, public_key.get())) {
        std::string headerAndPayload = data[0] + "." + data[1];
        std::string signature = helpers::base64UrlDecode(data[2]);
        if (1 == EVP_DigestVerifyUpdate(ctx.get(), headerAndPayload.c_str(), headerAndPayload.size())) {
            int result = EVP_DigestVerifyFinal(ctx.get(), (const unsigned char*)signature.c_str(), signature.size());
            verified = (result == 1);
        }
    }
    return verified;
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
        if (parts.size() >= 3) {
            if (verifySignature(parts)) {
                LOG_TRACE << "Verified";
                Json::Value json;
                Json::Reader reader;
                reader.parse(helpers::base64UrlDecode(parts[1]), json);
                std::string email = json["email"].asString();
                std::string exp = json["exp"].asString();
                if (isAuthorized(email)) {
                    req->session()->insert("exp", exp);
                    req->session()->insert("email", email);
                    std::string query = "UPDATE " + helpers::TablePrefix + "Users SET token = $1 WHERE email = $2";
                    auto clientPtr = drogon::app().getDbClient();
                    auto result = clientPtr->execSqlSync(query, exp, email);
                    LOG_TRACE << "Logged in";
                    auto resp = drogon::HttpResponse::newRedirectionResponse(req->getHeader("referer"));
                    callback(resp);
                    return;
                }
            }
        }
    }
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(drogon::k401Unauthorized);
    callback(res);
}

