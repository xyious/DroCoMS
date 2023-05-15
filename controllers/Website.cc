#include <iomanip>

#include "Website.h"
#include "helpers/helpers.h"
#include "../helpers/Constants.h"

website::website(std::vector<std::string> keywords, std::string language, std::string title, std::string content, std::string leftSidebarContent, std::string rightSidebarContent, std::string stylesheet) {
    this->keywords = keywords;
    this->language = language;
    this->title = title;
    this->content = "<div class='content-container'>" + content + "</div>";
    this->stylesheet = stylesheet;
    this->leftSidebarContent = "<div class='left-sidebar'><ul><li><h3><a href='" + helpers::BaseURL + "'>Home</a></h3></li>" + leftSidebarContent + "</ul></div>";
    if (rightSidebarContent.empty()) {
        rightSidebarContent = "&nbsp;";
    } else {
        rightSidebarContent = "<ul>" + rightSidebarContent + "</ul>";
    }
    this->rightSidebarContent = "<div class='right-sidebar'>" + rightSidebarContent + "</div>";
}

website::website() {}

void website::setContent(std::string content) {
    this->content = "<div class='content-container'>" + content + "</div>";
}

void website::setLanguage(std::string language) {
    this->language = language;
}

void website::setTitle(std::string title) {
    this->title = title;
}

void website::setLeftSidebarContent(std::string content) {
    this->leftSidebarContent = "<div class='left-sidebar'><ul><li><h3><a href='" + helpers::BaseURL + "'>Home</a></h3></li>" + leftSidebarContent + "</ul></div>";
}

void website::setRightSidebarContent(std::string content) {
    if (rightSidebarContent.empty()) {
        rightSidebarContent = "&nbsp;";
    } else {
        rightSidebarContent = "<ul>" + rightSidebarContent + "</ul>";
    }
    this->rightSidebarContent = "<div class='right-sidebar'>" + rightSidebarContent + "</div>";
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
    std::string result = ((std::string)"<link rel='stylesheet' href='" + path + "'>");
    return result;
}

std::string website::getPost(std::string url, std::string title, std::string subtitle, std::string content, std::string author, std::string timestamp) {
    std::string result = "<div class='post-container'><a href='" + helpers::BaseURL + "/Blog/";
    result.append(url + "'><h1>" + title + "</a></h1>");
    if (!subtitle.empty()) {
        result.append("<h4>" + subtitle + "</h4>");
    }
    result.append("<div class='post-content-container'>" + content);
    result.append("</div><div class='post-info-container'><div class='post-time'>Posted: on " + timestamp + "</div><div class='author-info'> by " + author + "</div></div></div>");
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
    this->page.append("<script async src='https://www.googletagmanager.com/gtag/js?id=" + helpers::AnalyticsId + "'></script><script>window.dataLayer = window.dataLayer || [];function gtag(){dataLayer.push(arguments);}gtag('js', new Date());gtag('config', '" + helpers::AnalyticsId + "');</script>" + getTitleTag() + getStyleTag() + BODYTAG + this->leftSidebarContent + this->rightSidebarContent + this->content + ENDTAG);
    resp->setStatusCode(drogon::HttpStatusCode::k200OK);
    resp->setBody(this->page);
    return resp;
}