#include "Website.h"
#include "../helpers/Constants.h"

website::website(std::vector<std::string> keywords, std::string languague, std::string title, std::string content, std::string stylesheet) {
    this->keywords = keywords;
    this->language = language;
    this->title = title;
    this->content = content;
    this->stylesheet = stylesheet;
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

std::string website::getStyleTag(std::string path = "") {
    if (path.empty()) {
        path = "/stylesheet.css";
    }
    std::string result = ((std::string)"<link rel='stylesheet' href='").append(path).append("'>");
    return result;
}

std::string website::getContent(std::string title, std::string subtitle, std::string content, std::string author, std::string timestamp, std::string tags) {
    std::string result = "<h1>";
    result.append(title).append("</h1>");
    if (!subtitle.empty()) {
        result.append("<h4>").append(subtitle).append("</h4>");
    }
    result.append(content);
    result.append("<span class='post-time'>").append(timestamp).append("</span></span class='author-info'>").append(author).append("</span>");
    return result;
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
    this->page.append(getStyleTag());
    this->page.append(BODYTAG);
    this->page.append(this->content);
    this->page.append(ENDTAG);
    resp->setBody(this->page);
    return resp;
}