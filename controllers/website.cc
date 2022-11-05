#include "website.h"
#include "../helpers/constants.h"

website::website(std::vector<std::string> keywords, std::string languague, std::string title, std::string content) {
    this->keywords = keywords;
    this->language = language;
    this->title = title;
    this->content = content;
}

std::string website::getTitle() {
    return this->title;
}

std::string website::getPage() {
    this->page = DOCTYPE;
    if (this->language == "de") {
        this->page.append(HTMLTAGDE);
    } else {
        this->page.append(HTMLTAGEN);
    }
    this->page.append(BODYTAG);
    this->page.append(this->content);
    this->page.append(ENDTAG);
    return this->page;
}