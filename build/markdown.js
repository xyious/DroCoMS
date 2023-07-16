document.addEventListener('DOMContentLoaded', () => { 
    document.getElementsByTagName('textarea')[0].addEventListener("input", queueUpdate);
});

class Preview {
    html = "";
    ul = false;
    ol = 0;

    get() {
        return this.html;
    }
    addLine(text, ul = false, ol = 0) {
        if (this.ul == false && ul == true) {
            text = "<ul>" + text;
            this.html += text;
            this.ul = true;
        }
        if (this.ol == 0 && ol == 1) {
            text = "<ol>" + text;
            this.html += text;
            this.ol = 1;
        }
        if (this.ol == ol - 1) {
            this.html += text;
            this.ol += 1;
        }
        this.html += text;
    }
    endText() {
        if (this.ul == true) {
            this.html += "</ul>";
        }
        if (this.ol > 0) {
            this.html += "</ol>";
        }
    }
    reset() {
        this.html = "";
    }
}

let doUpdate = 0;
let markdownPreview = new Preview();

function preview() {
    if (doUpdate > 5) {
        doUpdate = 0;
        let result = "";
        let lines = document.getElementsByTagName('textarea')[0].value.split("\n");
        markdownPreview.reset();
        lines.forEach((value) => {parseLine(value);});
        markdownPreview.endText();
        document.getElementById("preview").innerHTML = markdownPreview.get();
    }
    if (doUpdate > 0) {
        doUpdate += 1;
        setTimeout(preview, 5000);
    }
}

function queueUpdate() {
    doUpdate = 1;
    setTimeout(preview, 1000);
}

function parseLine(line) {
    let ul = false;
    let ol = 0;
    console.log(line);
    if (line.match(/\B\[([^\[\]]+)\]\(([^\)]+)\)\B/)) {
        line = line.replace(/\B\[([^\[\]]+)\]\(([^\)]+)\)\B/g, getLink);
    }
    if (line.startsWith(" - ")) {
        ul = true;
        line = "<li>" + line + "</li>";
    }
    if (line.startsWith(" [1-9]+\.")) {
        // check if we're inside <ol> and add if we are (if the number matches the last number +1
        ol = 1; // TODO: get the number
        let result = "<li>" + line + "</li>";
    }
    if (ol == 0 && ul == false) {
        line = "<p>" + line + "</p>";
    }
    markdownPreview.addLine(line, ul, ol);
}

function getLink(match, description, url, offset, string) {
    return "<a href='" + url + "'>" + description + "</a>";
}
