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
    addLine(text, ul, ol) {
        if (this.ul == false && ul == true) {
            text = "<ul>" + text;
            this.ul = true;
        }
        if (this.ol == ol - 1) {
            if (this.ol == 0) {
                text = "<ol>" + text;
            }
            this.ol += 1;
        }
        if (this.ul && ul == false) {
            this.ul = false;
            text = "</ul>" + text;
        }
        if (this.ol > 0 && ol == 0) {
            this.ol = 0;
            text = "</ol>" + text;
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
        this.ol = 0;
        this.ul = false;
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
    let h = false;
    console.log(line);
    // First we need to worry about everything that doesn't interfere with 
    // creating paragraphs (things that end up inside either paragraph or list or h1, etc.
    line = line.replace(/\B\[([^\[\]]+)\]\(([^\)]+)\)\B/g, getLink);
    line = line.replace(/\*\*([^\*]+)\*\*/g, "<b>$1</b>");
    line = line.replace(/\*([^\*]+)\*/g, "<i>$1</i>");
    if (line.startsWith(" - ")) {
        ul = true;
        ol = -1;
        line = "<li>" + line.substring(3) + "</li>";
    }
    // We want to have a ul that's *not* inside an existing list (either ul or ol)
    if (line.startsWith("- ")) {
        ul = true;
        line = "<li>" + line.substring(2) + "</li>";
    }
    let found = line.match(/^[0-9]+\. /);
    if (found) {
        // check if we're inside <ol> and add if we are (if the number matches the last number +1
        ol = found[0].match(/^[0-9]+\. /)[0];
        line = "<li>" + line.substring(ol.length) + "</li>";
    }
    if (line.startsWith("#")) {
        h = true;
        line = line.replace(/###### (.+)/, "<h6>$1</h6>");
        line = line.replace(/##### (.+)/, "<h5>$1</h5>");
        line = line.replace(/#### (.+)/, "<h4>$1</h4>");
        line = line.replace(/### (.+)/, "<h3>$1</h3>");
        line = line.replace(/## (.+)/, "<h2>$1</h2>");
        line = line.replace(/# (.+)/, "<h1>$1</h1>");
    }
    if (ol == 0 && ul == false && h == false) {
        line = "<p>" + line + "</p>";
    }
    markdownPreview.addLine(line, ul, ol);
}

function getLink(match, description, url, offset, string) {
    return "<a href='" + url + "'>" + description + "</a>";
}
