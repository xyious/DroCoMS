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

std::string website::getLanguage() {
    return this->language;
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

std::string website::getPost(std::string url, std::string title, std::string subtitle, std::string content, std::string author, std::string timestamp, std::vector<std::string> tags) {
    std::string result = "<div class='post-container'><a href='http://xyious.com/Blog/";
    result.append(url).append("'><h1>").append(title).append("</a></h1>");
    if (!subtitle.empty()) {
        result.append("<h4>").append(subtitle).append("</h4>");
    }
    result.append("<div class='content-container'>").append(content);
    result.append("</div><div class='post-info-container'><div class='post-time'>Posted: ").append(timestamp).append("</div><div class='author-info'> by ").append(author).append("</div></div></div>");
    return result;
}

std::shared_ptr<drogon::HttpResponse> website::getPage() {
    auto resp = drogon::HttpResponse::newHttpResponse();
    this->page = DOCTYPE;
    if (this->language == "de-DE") {
        this->page.append(HTMLTAGDE);
    } else {
        this->page.append(HTMLTAGEN);
    }
    this->page.append(getTitleTag()).append(getStyleTag()).append(BODYTAG).append(this->content).append(ENDTAG);
    resp->setBody(this->page);
    return resp;
}