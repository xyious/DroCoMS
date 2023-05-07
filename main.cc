#include <json/json.h>
#include <drogon/drogon.h>

#include "helpers/helpers.h"

std::string helpers::BaseURL;
std::string helpers::TablePrefix;
std::string helpers::AnalyticsId;

std::string getCustomValue(const Json::Value json, std::string key) {
    if (!json[key].empty())
    {
        return json[key].asString();
    }
    return "";
}

int main() {
    //Load config file
    drogon::app().loadConfigFile("config.json");
    auto &json = drogon::app().getCustomConfig();
    helpers::BaseURL = getCustomValue(json, "base_url");
    helpers::TablePrefix = getCustomValue(json, "table_prefix");
    helpers::AnalyticsId = getCustomValue(json, "analytics_id");
    LOG_TRACE << helpers::BaseURL;
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
