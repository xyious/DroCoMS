#include <json/json.h>
#include <drogon/drogon.h>

#include "helpers/helpers.h"

std::string helpers::BaseURL;

int main() {
    //Load config file
    drogon::app().loadConfigFile("config.json");
    std::string url;
    auto &json = drogon::app().getCustomConfig();
    if (!json["base_url"].empty())
    {
        url = json["base_url"].asString();
    }
    helpers::BaseURL = url;
    LOG_TRACE << helpers::BaseURL;
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
