#include <iomanip>

#include "Website.h"
#include "helpers/helpers.h"
#include "../helpers/Constants.h"

Website::Website(std::vector<std::string> keywords, std::string language, std::string title, std::string content, std::string leftSidebarContent, std::string rightSidebarContent, std::string stylesheet, std::vector<std::string> scripts) {
    this->keywords = keywords;
    this->language = language;
    this->title = title;
    this->setLeftSidebarContent(leftSidebarContent);
    this->setRightSidebarContent(rightSidebarContent);
    this->setContent(content);
    if (this->stylesheet.length() == 0) {
        this->stylesheet = "<link rel='stylesheet' href='/stylesheet.css'>";
    } else {
       this->stylesheet = "<link rel='stylesheet' href='" + stylesheet + "'>";
    }
    this->scripts = scripts;
}

Website::Website() {
    this->stylesheet = "<link rel='stylesheet' href='/stylesheet.css'>";
}

void Website::setContent(std::string content) {
    this->content = "<div class='content-container'>" + content + "</div>";
}

std::string Website::getLanguage() {
    return this->language;
}

void Website::setLanguage(std::string language) {
    this->language = language;
}

std::string Website::getTitle() {
    return this->title;
}

void Website::setTitle(std::string title) {
    this->title = title;
}

void Website::setLeftSidebarContent(std::string content) {
    this->leftSidebarContent = "<nav class='left-sidebar'><ul><li><h3><a href='" + helpers::BaseURL + "'>Home</a></h3></li>" + content + "</ul></nav>";
}

void Website::setRightSidebarContent(std::string content) {
    if (content.empty()) {
        content = "&nbsp;";
    } else {
        content = "<ul>" + content + "</ul>";
    }
    this->rightSidebarContent = "<div class='right-sidebar'>" + rightSidebarContent + "</div>";
}

std::string Website::getTitleTag() {
    return "<title>" + this->title + "</title>";
}

std::string Website::getKeywordsTag() {
    if (this->keywords.size() < 1) {
        return "";
    }
    std::string tag = "<meta name=\"keywords\" content=\"";
    for (auto keyword : this->keywords) {
        tag.append(keyword + ", ");
    }
    // I don't like trailing commas, even if it's unlikely to be a problem
    tag = tag.substr(0, tag.length() - 2);
    tag.append("\">");
    LOG_TRACE << tag;
    return tag;
}

std::string Website::getPost(std::string url, std::string title, std::string subtitle, std::string content, std::string author, std::string timestamp) {
    std::string result = "<div class='post-container'><a href='" + helpers::BaseURL + "/Blog/";
    result.append(url + "'><h1>" + title + "</a></h1>");
    if (!subtitle.empty()) {
        result.append("<h4>" + subtitle + "</h4>");
    }
    result.append("<div class='post-content-container'>" + content);
    result.append("</div><div class='post-info-container'><div class='post-time'>Posted: on " + timestamp + "</div><div class='author-info'> by " + author + "</div></div></div>");
    return result;
}

std::shared_ptr<drogon::HttpResponse> Website::getPage() {
    auto resp = drogon::HttpResponse::newHttpResponse();
    this->page = DOCTYPE;
    if (this->language == "de-DE") {
        this->page.append(HTMLTAGDE);
    } else {
        this->page.append(HTMLTAGEN);
    }
    this->page.append(getKeywordsTag());
    for (auto script : this->scripts) {
        LOG_TRACE << script;
        this->page.append("<script async src='" + script + "'></script>");
    }
    this->page.append("<script async src='https://www.googletagmanager.com/gtag/js?id=" + helpers::AnalyticsId);
    this->page.append("'></script><script>window.dataLayer = window.dataLayer || [];function gtag(){dataLayer.push(arguments);}gtag('js', new Date());gtag('config', '" + helpers::AnalyticsId + "');</script>");
    this->page.append(getTitleTag() + this->stylesheet + BODYTAG + this->leftSidebarContent + this->rightSidebarContent + this->content + ENDTAG);
    resp->setStatusCode(drogon::HttpStatusCode::k200OK);
    resp->setBody(this->page);
    return resp;
}
