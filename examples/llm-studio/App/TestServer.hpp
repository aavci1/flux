#pragma once

#include "ScreenCapture.hpp"
#include "StateSerializer.hpp"
#include "ActionDispatcher.hpp"
#include <thread>
#include <atomic>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace llm_studio {

class TestServer {
public:
    TestServer(AppState& state, ScreenCapture& capture, int port = 8435)
        : dispatcher_(state), state_(state), capture_(capture), port_(port) {}

    ~TestServer() { stop(); }

    void start() {
        if (running_.exchange(true)) return;
        thread_ = std::thread([this]() { run(); });
        std::cout << "\n  TEST SERVER: http://localhost:" << port_ << "\n" << std::endl;
    }

    void stop() {
        running_ = false;
        if (serverFd_ >= 0) {
            shutdown(serverFd_, SHUT_RDWR);
            ::close(serverFd_);
            serverFd_ = -1;
        }
        if (thread_.joinable()) thread_.join();
    }

private:
    void run() {
        serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd_ < 0) { std::cerr << "TestServer: socket failed\n"; return; }

        int opt = 1;
        setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);

        if (bind(serverFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "TestServer: bind failed on port " << port_ << "\n";
            ::close(serverFd_);
            serverFd_ = -1;
            return;
        }

        listen(serverFd_, 8);

        while (running_) {
            struct sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientFd = accept(serverFd_, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientFd < 0) continue;

            handleClient(clientFd);
            ::close(clientFd);
        }
    }

    void handleClient(int fd) {
        char buf[8192];
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) return;
        buf[n] = '\0';

        std::string request(buf, n);
        std::string method, path;
        std::istringstream reqStream(request);
        reqStream >> method >> path;

        if (method == "GET" && path == "/") {
            sendHtml(fd);
        } else if (method == "GET" && path.starts_with("/screenshot")) {
            sendScreenshot(fd);
        } else if (method == "GET" && path == "/state") {
            sendState(fd);
        } else if (method == "POST" && path == "/action") {
            std::string body;
            auto bodyPos = request.find("\r\n\r\n");
            if (bodyPos != std::string::npos) {
                body = request.substr(bodyPos + 4);
            }
            sendAction(fd, body);
        } else {
            std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
            send(fd, resp.data(), resp.size(), 0);
        }
    }

    void sendResponse(int fd, const std::string& status, const std::string& contentType,
                      const void* body, size_t bodyLen) {
        std::string header = "HTTP/1.1 " + status + "\r\n"
            "Content-Type: " + contentType + "\r\n"
            "Content-Length: " + std::to_string(bodyLen) + "\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Connection: close\r\n"
            "\r\n";
        send(fd, header.data(), header.size(), 0);
        if (bodyLen > 0) {
            send(fd, body, bodyLen, 0);
        }
    }

    void sendHtml(int fd) {
        std::string html = buildControlPanel();
        sendResponse(fd, "200 OK", "text/html; charset=utf-8", html.data(), html.size());
    }

    void sendScreenshot(int fd) {
        auto png = capture_.getPng();
        if (png.empty()) {
            std::string msg = "No screenshot available yet";
            sendResponse(fd, "503 Service Unavailable", "text/plain", msg.data(), msg.size());
            return;
        }
        sendResponse(fd, "200 OK", "image/png", png.data(), png.size());
    }

    void sendState(int fd) {
        std::string json = serializeState(state_);
        sendResponse(fd, "200 OK", "application/json", json.data(), json.size());
    }

    void sendAction(int fd, const std::string& body) {
        std::string action = body;
        // Trim whitespace
        while (!action.empty() && (action.back() == '\n' || action.back() == '\r' || action.back() == ' '))
            action.pop_back();
        std::string result = dispatcher_.dispatch(action);
        sendResponse(fd, "200 OK", "application/json", result.data(), result.size());
    }

    std::string buildControlPanel() {
        return R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>LLM Studio - Test Control Panel</title>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: #0a0a0a; color: #e0e0e0;
    display: flex; flex-direction: column; height: 100vh;
  }
  header {
    padding: 12px 20px; background: #141414; border-bottom: 1px solid #2a2a2a;
    display: flex; align-items: center; gap: 16px;
  }
  header h1 { font-size: 16px; color: #618FFF; }
  header .status { font-size: 12px; color: #5abf80; }
  .main { display: flex; flex: 1; overflow: hidden; }
  .panel {
    border-right: 1px solid #2a2a2a; width: 320px; overflow-y: auto;
    padding: 16px; flex-shrink: 0;
  }
  .screenshot-area { flex: 1; display: flex; flex-direction: column; align-items: center; justify-content: center; padding: 16px; overflow: auto; background: #111; }
  .screenshot-area img { max-width: 100%; max-height: 100%; border: 1px solid #2a2a2a; border-radius: 4px; }
  .section { margin-bottom: 20px; }
  .section h3 { font-size: 11px; text-transform: uppercase; color: #7a7a7a; letter-spacing: 0.5px; margin-bottom: 8px; }
  .btn-grid { display: flex; flex-wrap: wrap; gap: 6px; }
  .btn {
    padding: 6px 12px; font-size: 12px; border: 1px solid #3a3a3a; background: #1f1f1f;
    color: #e0e0e0; border-radius: 4px; cursor: pointer; transition: background 0.15s;
  }
  .btn:hover { background: #2a2a2a; }
  .btn.active { background: #618FFF22; border-color: #618FFF; color: #618FFF; }
  .btn.accent { background: #618FFF; color: #fff; border-color: #618FFF; }
  .btn.danger { background: #e64d4d33; border-color: #e64d4d; color: #e64d4d; }
  input[type="text"] {
    width: 100%; padding: 6px 10px; font-size: 12px; background: #1a1a1a;
    border: 1px solid #3a3a3a; color: #e0e0e0; border-radius: 4px; margin-bottom: 6px;
  }
  input[type="text"]:focus { outline: none; border-color: #618FFF; }
  .state-panel {
    border-top: 1px solid #2a2a2a; max-height: 240px; overflow-y: auto;
    padding: 12px 20px; font-size: 11px; font-family: 'SF Mono', Monaco, monospace;
    background: #0d0d0d; white-space: pre-wrap; color: #8a8a8a;
  }
  .state-panel .key { color: #618FFF; }
  .state-panel .str { color: #5abf80; }
  .state-panel .num { color: #e6a64d; }
  .state-panel .bool { color: #e64d4d; }
  #log {
    font-size: 11px; font-family: monospace; max-height: 100px; overflow-y: auto;
    background: #0d0d0d; padding: 8px; border-radius: 4px; margin-top: 8px; color: #7a7a7a;
  }
</style>
</head>
<body>
<header>
  <h1>LLM Studio Test Panel</h1>
  <span class="status" id="connStatus">Connected</span>
  <span style="flex:1"></span>
  <span style="font-size:11px;color:#7a7a7a" id="frameInfo">--</span>
</header>
<div class="main">
  <div class="panel">
    <div class="section">
      <h3>Navigation</h3>
      <div class="btn-grid">
        <button class="btn" onclick="act('navigate:CHAT')">Chat</button>
        <button class="btn" onclick="act('navigate:IMAGE')">Image</button>
        <button class="btn" onclick="act('navigate:HUB')">Hub</button>
        <button class="btn" onclick="act('navigate:SETTINGS')">Settings</button>
      </div>
    </div>

    <div class="section">
      <h3>Sidebar</h3>
      <div class="btn-grid">
        <button class="btn" onclick="act('toggle_sidebar')">Toggle Sidebar</button>
        <button class="btn" onclick="act('select_model:0')">Model 0</button>
        <button class="btn" onclick="act('select_model:1')">Model 1</button>
        <button class="btn" onclick="act('select_model:2')">Model 2</button>
      </div>
    </div>

    <div class="section">
      <h3>Chat</h3>
      <input type="text" id="chatInput" placeholder="Type a message..." />
      <div class="btn-grid">
        <button class="btn accent" onclick="typeAndSend()">Send</button>
        <button class="btn" onclick="act('clear_chat')">Clear</button>
        <button class="btn" onclick="act('show_clear_dialog')">Show Clear Dialog</button>
        <button class="btn" onclick="act('dismiss_clear_dialog')">Dismiss Dialog</button>
        <button class="btn danger" onclick="act('confirm_clear')">Confirm Clear</button>
      </div>
    </div>

    <div class="section">
      <h3>Hub</h3>
      <input type="text" id="hubInput" placeholder="Search query..." />
      <div class="btn-grid">
        <button class="btn accent" onclick="hubSearch()">Search</button>
      </div>
    </div>

    <div class="section">
      <h3>Log</h3>
      <div id="log"></div>
    </div>
  </div>

  <div class="screenshot-area">
    <img id="screenshot" src="/screenshot" alt="App Screenshot" />
  </div>
</div>
<div class="state-panel" id="statePanel">Loading state...</div>

<script>
function log(msg) {
  const el = document.getElementById('log');
  const t = new Date().toLocaleTimeString();
  el.textContent = `[${t}] ${msg}\n` + el.textContent;
  if (el.textContent.length > 5000) el.textContent = el.textContent.slice(0, 5000);
}

async function act(action) {
  log(`Action: ${action}`);
  try {
    const res = await fetch('/action', { method: 'POST', body: action });
    const data = await res.json();
    log(`Result: ${JSON.stringify(data)}`);
    setTimeout(refreshAll, 200);
  } catch (e) {
    log(`Error: ${e.message}`);
  }
}

function typeAndSend() {
  const input = document.getElementById('chatInput').value;
  if (!input) return;
  act('type_chat:' + input).then(() => {
    setTimeout(() => act('send_message'), 100);
  });
  document.getElementById('chatInput').value = '';
}

function hubSearch() {
  const q = document.getElementById('hubInput').value;
  if (!q) return;
  act('search_hub:' + q);
}

function refreshScreenshot() {
  const img = document.getElementById('screenshot');
  img.src = '/screenshot?t=' + Date.now();
}

async function refreshState() {
  try {
    const res = await fetch('/state');
    const state = await res.json();
    const el = document.getElementById('statePanel');
    el.innerHTML = syntaxHighlight(JSON.stringify(state, null, 2));
    document.getElementById('frameInfo').textContent =
      `View: ${state.currentView} | Msgs: ${state.messageCount} | Models: ${state.installedModelCount}`;
  } catch(e) {
    document.getElementById('connStatus').textContent = 'Disconnected';
    document.getElementById('connStatus').style.color = '#e64d4d';
  }
}

function syntaxHighlight(json) {
  return json.replace(/("(\\u[a-zA-Z0-9]{4}|\\[^u]|[^\\"])*"(\s*:)?|\b(true|false|null)\b|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?)/g,
    function(match) {
      let cls = 'num';
      if (/^"/.test(match)) {
        cls = /:$/.test(match) ? 'key' : 'str';
      } else if (/true|false/.test(match)) {
        cls = 'bool';
      }
      return '<span class="' + cls + '">' + match + '</span>';
    });
}

function refreshAll() {
  refreshScreenshot();
  refreshState();
}

setInterval(refreshAll, 1500);
refreshAll();
</script>
</body>
</html>)HTML";
    }

    ActionDispatcher dispatcher_;
    AppState& state_;
    ScreenCapture& capture_;
    int port_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    int serverFd_ = -1;
};

} // namespace llm_studio
