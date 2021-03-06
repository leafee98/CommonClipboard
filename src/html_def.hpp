#include <string>

std::string html_code = R"(
<!DOCTYPE html>

<html>

    <head>
        <title>simple web common clipboard</title>
        <meta charset='utf-8'>
    </head>

    <body style="align-items: center; display: flex;">
        <div style="margin: auto; text-align: center;">
            <h1>Common clipboard</h1>
            <hr />

            <div>
                <b>Server Address:</b>
                <input id="id_uri"/>
                <button id="id_connect">Connect</button>
            </div>

            <div style="margin-top: 8px; margin-bottom: 8px">
                <b>Server State:</b>
                <i id="id_server_state">disconnected</i>
                <button id="id_save">Save</button>
                <button id="id_load">Load</button>
                <br />
            </div>

            <textarea id="id_content" type="text" style="padding: 8px; width: 800px; height:350px;"></textarea>
        </div>
    </body>
</html>

<script>
    // this block writen the functions

    var op_cookie = {};
    op_cookie.parseCookie = function(cookie_str) {
        let res = {};
        if (! cookie_str.endsWith(';'))
            cookie_str += ';';
        
        while (cookie_str.length > 1) {
            let ie = cookie_str.indexOf('=');
            let is = cookie_str.indexOf(';');

            let key = cookie_str.substring(0, ie);
            let value = cookie_str.substring(ie + 1, is);
            res[key.trim()] = value;

            cookie_str = cookie_str.substring(is + 1);
        }
        return res;
    }
    op_cookie.get = function(name) {
        return this.parseCookie(document.cookie)[name];
    }
    op_cookie.set = function(name, value, expire_seconds) {
        let expire_date;
        if (expire_seconds == undefined) {
            expire_date = new Date(Date.now() + 7 * 24 * 3600 * 1000);
        } else {
            expire_date = new Date(Date.now() + expire_seconds);
        }

        let tmp_cookie = name + "="  + value + "; Expires=" + expire_date + ";SameSite=Strict"
        document.cookie = tmp_cookie;
    }

// ===================================================================


    // change the default uri of webpage here.
    var host = location.hostname
    var default_uri = "ws://" + host + ":1234";

    let cookie_uri = op_cookie.get("uri");
    if (cookie_uri == undefined) {
        id_uri.value = default_uri;
    } else {
        id_uri.value = cookie_uri;
        op_cookie.set("uri", cookie_uri); // refresh expires date.
    }

    var ws = null;

    id_connect.onclick = function () {
        op_cookie.set("uri", id_uri.value);

        ws = new WebSocket(id_uri.value);
        ws.onopen = function(ev) {
            console.log("[websocket connected]");
            id_server_state.textContent = "connected";
        };
        ws.onclose = function(ev) {
            console.log("[websocket disconnected]");
            id_server_state.textContent = "disconnected";
        };
        ws.onmessage = function(ev) {
            console.log("[message received]");
            console.log(ev.data);
            id_content.value = ev.data;
        }
    }

    id_save.onclick = function () {
        ws.send("SAVE");
        ws.send(id_content.value);
    }
    
    id_load.onclick = function() {
        ws.send("LOAD");
    }
</script>
)";

std::string header_code_pre = R"(HTTP/2 200 OK
Content-type: text/html; charset=utf-8
Connection: close
Server: custom)";

std::string header_code = header_code_pre + "\nContent-Length: " +
                          std::to_string(html_code.size()) + "\n\n";
const std::string response_str = header_code + html_code;
