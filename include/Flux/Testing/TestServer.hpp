#pragma once

#include <Flux/Core/Window.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <thread>
#include <atomic>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace flux {

class TestServer {
public:
    TestServer(Window& window, int port = 8435)
        : window_(window), port_(port) {}

    ~TestServer() { stop(); }

    void start() {
        if (running_.exchange(true)) return;
        thread_ = std::thread([this]() { run(); });
        std::cout << "\n  FLUX TEST SERVER: http://localhost:" << port_ << "\n"
                  << "  Endpoints: GET /ui  GET /screenshot  POST /click  POST /type  POST /key  POST /scroll  POST /hover\n" << std::endl;
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

    // --- Snapshot management (called from main thread post-render) ---

    void updateUITreeJSON(const std::string& json) {
        std::lock_guard<std::mutex> lock(snapshotMutex_);
        uiTreeJSON_ = json;
    }

    void updateScreenshot(std::vector<uint8_t> png) {
        std::lock_guard<std::mutex> lock(snapshotMutex_);
        screenshot_ = std::move(png);
    }

    void signalFrameComplete() {
        std::lock_guard<std::mutex> lock(frameMutex_);
        ++frameCounter_;
        frameCV_.notify_all();
    }

    // --- Synthetic event queue (read by main thread) ---

    struct SyntheticEvent {
        enum Type { Click, TextInput, KeyPress, Scroll, Hover };
        Type type;
        float x = 0, y = 0;
        float deltaX = 0, deltaY = 0;
        std::string text;
        int keyCode = 0;
    };

    std::vector<SyntheticEvent> drainEvents() {
        std::lock_guard<std::mutex> lock(eventMutex_);
        std::vector<SyntheticEvent> out;
        out.swap(pendingEvents_);
        return out;
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
        char buf[16384];
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) return;
        buf[n] = '\0';

        std::string request(buf, n);
        std::string method, path;
        std::istringstream reqStream(request);
        reqStream >> method >> path;

        std::string body;
        auto bodyPos = request.find("\r\n\r\n");
        if (bodyPos != std::string::npos) {
            body = request.substr(bodyPos + 4);
        }

        if (method == "OPTIONS") {
            sendCorsOk(fd);
        } else if (method == "GET" && path == "/ui") {
            sendUITree(fd);
        } else if (method == "GET" && path.starts_with("/screenshot")) {
            sendScreenshot(fd);
        } else if (method == "POST" && path == "/click") {
            handleClick(fd, body);
        } else if (method == "POST" && path == "/type") {
            handleType(fd, body);
        } else if (method == "POST" && path == "/key") {
            handleKey(fd, body);
        } else if (method == "POST" && path == "/scroll") {
            handleScroll(fd, body);
        } else if (method == "POST" && path == "/hover") {
            handleHover(fd, body);
        } else {
            std::string msg = R"({"error":"not found"})";
            sendResponse(fd, "404 Not Found", "application/json", msg.data(), msg.size());
        }
    }

    // --- Response helpers ---

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

    void sendCorsOk(int fd) {
        sendResponse(fd, "204 No Content", "text/plain", nullptr, 0);
    }

    // --- GET handlers ---

    void sendUITree(int fd) {
        std::string json;
        {
            std::lock_guard<std::mutex> lock(snapshotMutex_);
            json = uiTreeJSON_;
        }
        if (json.empty()) json = "{}";
        sendResponse(fd, "200 OK", "application/json", json.data(), json.size());
    }

    void sendScreenshot(int fd) {
        std::vector<uint8_t> png;
        {
            std::lock_guard<std::mutex> lock(snapshotMutex_);
            png = screenshot_;
        }
        if (png.empty()) {
            std::string msg = "No screenshot available";
            sendResponse(fd, "503 Service Unavailable", "text/plain", msg.data(), msg.size());
            return;
        }
        sendResponse(fd, "200 OK", "image/png", png.data(), png.size());
    }

    // --- POST handlers ---

    void handleClick(int fd, const std::string& body) {
        float x = 0, y = 0;
        parseFloat(body, "x", x);
        parseFloat(body, "y", y);

        uint64_t frameBefore = currentFrame();
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            pendingEvents_.push_back({SyntheticEvent::Click, x, y, 0, 0, "", 0});
        }
        window_.requestRedraw();
        waitForFrame(frameBefore);

        std::string resp = R"({"ok":true,"action":"click","x":)" + std::to_string(x) + R"(,"y":)" + std::to_string(y) + "}";
        sendResponse(fd, "200 OK", "application/json", resp.data(), resp.size());
    }

    void handleType(int fd, const std::string& body) {
        std::string text = parseString(body, "text");

        uint64_t frameBefore = currentFrame();
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            pendingEvents_.push_back({SyntheticEvent::TextInput, 0, 0, 0, 0, text, 0});
        }
        window_.requestRedraw();
        waitForFrame(frameBefore);

        std::string resp = R"({"ok":true,"action":"type"})";
        sendResponse(fd, "200 OK", "application/json", resp.data(), resp.size());
    }

    void handleKey(int fd, const std::string& body) {
        std::string keyStr = parseString(body, "key");
        int keyCode = keyNameToCode(keyStr);

        uint64_t frameBefore = currentFrame();
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            pendingEvents_.push_back({SyntheticEvent::KeyPress, 0, 0, 0, 0, "", keyCode});
        }
        window_.requestRedraw();
        waitForFrame(frameBefore);

        std::string resp = R"({"ok":true,"action":"key","key":")" + escapeJson(keyStr) + "\"}";
        sendResponse(fd, "200 OK", "application/json", resp.data(), resp.size());
    }

    void handleScroll(int fd, const std::string& body) {
        float x = 0, y = 0, dx = 0, dy = 0;
        parseFloat(body, "x", x);
        parseFloat(body, "y", y);
        parseFloat(body, "deltaX", dx);
        parseFloat(body, "deltaY", dy);

        uint64_t frameBefore = currentFrame();
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            pendingEvents_.push_back({SyntheticEvent::Scroll, x, y, dx, dy, "", 0});
        }
        window_.requestRedraw();
        waitForFrame(frameBefore);

        std::string resp = R"({"ok":true,"action":"scroll"})";
        sendResponse(fd, "200 OK", "application/json", resp.data(), resp.size());
    }

    void handleHover(int fd, const std::string& body) {
        float x = 0, y = 0;
        parseFloat(body, "x", x);
        parseFloat(body, "y", y);

        uint64_t frameBefore = currentFrame();
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            pendingEvents_.push_back({SyntheticEvent::Hover, x, y, 0, 0, "", 0});
        }
        window_.requestRedraw();
        waitForFrame(frameBefore);

        std::string resp = R"({"ok":true,"action":"hover"})";
        sendResponse(fd, "200 OK", "application/json", resp.data(), resp.size());
    }

    // --- Frame synchronization ---

    uint64_t currentFrame() {
        std::lock_guard<std::mutex> lock(frameMutex_);
        return frameCounter_;
    }

    void waitForFrame(uint64_t afterFrame) {
        std::unique_lock<std::mutex> lock(frameMutex_);
        frameCV_.wait_for(lock, std::chrono::milliseconds(2000),
            [&]() { return frameCounter_ > afterFrame; });
    }

    // --- Minimal JSON parsing helpers ---

    static void parseFloat(const std::string& json, const std::string& key, float& out) {
        std::string needle = "\"" + key + "\"";
        auto pos = json.find(needle);
        if (pos == std::string::npos) return;
        pos = json.find(':', pos + needle.size());
        if (pos == std::string::npos) return;
        pos++;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try { out = std::stof(json.substr(pos)); } catch (...) {}
    }

    static std::string parseString(const std::string& json, const std::string& key) {
        std::string needle = "\"" + key + "\"";
        auto pos = json.find(needle);
        if (pos == std::string::npos) return "";
        pos = json.find(':', pos + needle.size());
        if (pos == std::string::npos) return "";
        pos = json.find('"', pos + 1);
        if (pos == std::string::npos) return "";
        pos++;
        auto end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }

    static std::string escapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out += c;
            }
        }
        return out;
    }

    static int keyNameToCode(const std::string& name) {
        if (name == "Enter" || name == "Return") return static_cast<int>(Key::Enter);
        if (name == "Tab") return static_cast<int>(Key::Tab);
        if (name == "Backspace") return static_cast<int>(Key::Backspace);
        if (name == "Escape") return static_cast<int>(Key::Escape);
        if (name == "Space") return static_cast<int>(Key::Space);
        if (name == "Left") return static_cast<int>(Key::Left);
        if (name == "Right") return static_cast<int>(Key::Right);
        if (name == "Up") return static_cast<int>(Key::Up);
        if (name == "Down") return static_cast<int>(Key::Down);
        if (name == "Home") return static_cast<int>(Key::Home);
        if (name == "End") return static_cast<int>(Key::End);
        if (name == "PageUp") return static_cast<int>(Key::PageUp);
        if (name == "PageDown") return static_cast<int>(Key::PageDown);
        if (name == "Delete") return static_cast<int>(Key::Delete);
        if (name == "Insert") return static_cast<int>(Key::Insert);
        if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z') {
            return static_cast<int>(Key::A) + (name[0] - 'A');
        }
        if (name.size() == 1 && name[0] >= 'a' && name[0] <= 'z') {
            return static_cast<int>(Key::A) + (name[0] - 'a');
        }
        if (name.size() == 1 && name[0] >= '0' && name[0] <= '9') {
            if (name[0] == '0') return static_cast<int>(Key::Num0);
            return static_cast<int>(Key::Num1) + (name[0] - '1');
        }
        return 0;
    }

    // --- Data ---

    Window& window_;
    int port_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    int serverFd_ = -1;

    std::mutex snapshotMutex_;
    std::string uiTreeJSON_;
    std::vector<uint8_t> screenshot_;

    std::mutex eventMutex_;
    std::vector<SyntheticEvent> pendingEvents_;

    std::mutex frameMutex_;
    std::condition_variable frameCV_;
    uint64_t frameCounter_ = 0;

public:
    // --- UI tree serialization (static utility) ---

    static std::string serializeUITree(const LayoutNode& root) {
        std::string out;
        out.reserve(4096);
        serializeNode(root, "0", out);
        return out;
    }

private:
    static void serializeNode(const LayoutNode& node, const std::string& id, std::string& out) {
        out += '{';

        out += "\"id\":\"";
        out += id;
        out += "\",\"type\":\"";
        out += escapeJson(node.view.getTypeName());
        out += "\",\"bounds\":{\"x\":";
        appendFloat(out, node.bounds.x);
        out += ",\"y\":";
        appendFloat(out, node.bounds.y);
        out += ",\"w\":";
        appendFloat(out, node.bounds.width);
        out += ",\"h\":";
        appendFloat(out, node.bounds.height);
        out += '}';

        std::string text = node.view.getTextContent();
        if (!text.empty()) {
            out += ",\"text\":\"";
            out += escapeJson(text);
            out += '"';
        }

        std::string value = node.view.getAccessibleValue();
        if (!value.empty()) {
            out += ",\"value\":\"";
            out += escapeJson(value);
            out += '"';
        }

        if (node.view.isInteractive()) {
            out += ",\"interactive\":true";
        }

        if (node.view.canBeFocused()) {
            out += ",\"focusable\":true";
            std::string fk = node.view.getFocusKey();
            if (!fk.empty()) {
                out += ",\"focusKey\":\"";
                out += escapeJson(fk);
                out += '"';
            }
        }

        if (!node.children.empty()) {
            out += ",\"children\":[";
            for (size_t i = 0; i < node.children.size(); i++) {
                if (i > 0) out += ',';
                std::string childId = id + "/" + std::to_string(i);
                serializeNode(node.children[i], childId, out);
            }
            out += ']';
        }

        out += '}';
    }

    static void appendFloat(std::string& out, float v) {
        char buf[32];
        int n = std::snprintf(buf, sizeof(buf), "%.1f", v);
        out.append(buf, n);
    }
};

} // namespace flux
