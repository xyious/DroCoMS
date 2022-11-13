#include "Website.h"
#include "../helpers/Constants.h"

website::website(std::vector<std::string> keywords, std::string languague, std::string title, std::string content) {
    this->keywords = keywords;
    this->language = language;
    this->title = title;
    this->content = content;
}

std::string website::getTitle() {
    return this->title;
}

std::string website::getTitleTag() {
    std::string title = "<title>";
    title.append(this->title);
    title.append("</title>");
    return title;
}

std::shared_ptr<drogon::HttpResponse> website::getPage() {
    auto resp = drogon::HttpResponse::newHttpResponse();
    this->page = DOCTYPE;
    if (this->language == "de") {
        this->page.append(HTMLTAGDE);
    } else {
        this->page.append(HTMLTAGEN);
    }
    this->page.append(getTitleTag());
    this->page.append(BODYTAG);
    this->page.append(this->content);
    this->page.append(ENDTAG);
    resp->setBody(this->page);
    return resp;
}