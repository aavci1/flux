#pragma once

#include <Flux/Core/Window.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Platform/EventLoopWake.hpp>
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
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

namespace flux {

class TestServer {
public:
    /// Binary request: magic(4) + version(2) + opcode(2) + bodyLen(4) + body
    static constexpr uint32_t kFluxTestMagic = 0x58554C46; // 'FLUX' little-endian bytes

    enum class Op : uint16_t {
        GetUi = 1,
        GetScreenshot = 2,
        Click = 3,
        Type = 4,
        Key = 5,
        Scroll = 6,
        Hover = 7,
        Drag = 8,
    };

    TestServer(Window& window, int tcpPort = 8435, std::string unixSocketPath = {})
        : window_(window), tcpPort_(tcpPort), unixSocketPath_(std::move(unixSocketPath)) {}

    ~TestServer() { stop(); }

    void start() {
        if (running_.exchange(true)) return;
        thread_ = std::thread([this]() { run(); });
        if (!unixSocketPath_.empty()) {
            std::cout << "\n  FLUX TEST IPC (unix): " << unixSocketPath_ << "\n"
                      << "  Binary ops: GetUi GetScreenshot Click Type Key Scroll Hover Drag\n" << std::endl;
        } else {
            std::cout << "\n  FLUX TEST IPC (tcp): localhost:" << tcpPort_ << "\n"
                      << "  Binary ops: GetUi GetScreenshot Click Type Key Scroll Hover Drag\n" << std::endl;
        }
    }

    void stop() {
        running_ = false;
        {
            std::lock_guard<std::mutex> lock(screenshotWaitMutex_);
            screenshotCV_.notify_all();
        }
        if (serverFd_ >= 0) {
            shutdown(serverFd_, SHUT_RDWR);
            ::close(serverFd_);
            serverFd_ = -1;
        }
        if (thread_.joinable()) thread_.join();
        if (!unixSocketPath_.empty()) {
            ::unlink(unixSocketPath_.c_str());
        }
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

    bool needsScreenshotCapture() const {
        return screenshotReqSeq_.load(std::memory_order_acquire) >
               screenshotDoneSeq_.load(std::memory_order_acquire);
    }

    void notifyScreenshotCaptured() {
        screenshotDoneSeq_.store(screenshotReqSeq_.load(std::memory_order_acquire), std::memory_order_release);
        std::lock_guard<std::mutex> lock(screenshotWaitMutex_);
        screenshotCV_.notify_all();
    }

    void signalFrameComplete() {
        std::lock_guard<std::mutex> lock(frameMutex_);
        ++frameCounter_;
        renderedSeq_ = processedSeq_;
        frameCV_.notify_all();
    }

    // --- Synthetic event queue (read by main thread) ---

    struct SyntheticEvent {
        enum Type { Click, TextInput, KeyPress, Scroll, Hover, MouseDown, MouseUp };
        Type type;
        float x = 0, y = 0;
        float deltaX = 0, deltaY = 0;
        std::string text;
        int keyCode = 0;
        uint64_t seq = 0;
    };

    std::vector<SyntheticEvent> drainEvents() {
        std::lock_guard<std::mutex> lock(eventMutex_);
        std::vector<SyntheticEvent> out;
        out.swap(pendingEvents_);
        return out;
    }

    void markEventsProcessed(const std::vector<SyntheticEvent>& events) {
        if (events.empty()) return;
        uint64_t maxSeq = 0;
        for (auto& e : events) {
            if (e.seq > maxSeq) maxSeq = e.seq;
        }
        if (maxSeq > processedSeq_) processedSeq_ = maxSeq;
    }

private:
    static bool readFull(int fd, void* dest, size_t n) {
        auto* p = static_cast<uint8_t*>(dest);
        size_t got = 0;
        while (got < n) {
            ssize_t r = recv(fd, p + got, n - got, 0);
            if (r <= 0) return false;
            got += static_cast<size_t>(r);
        }
        return true;
    }

    static bool sendAll(int fd, const void* data, size_t n) {
        const auto* p = static_cast<const uint8_t*>(data);
        size_t sent = 0;
        while (sent < n) {
            ssize_t w = send(fd, p + sent, n - sent, 0);
            if (w <= 0) return false;
            sent += static_cast<size_t>(w);
        }
        return true;
    }

    void sendBinaryResponse(int fd, uint8_t status, uint8_t payloadType, const void* body, size_t bodyLen) {
        uint8_t hdr[8];
        hdr[0] = status;
        hdr[1] = payloadType;
        hdr[2] = 0;
        hdr[3] = 0;
        uint32_t len32 = static_cast<uint32_t>(bodyLen);
        std::memcpy(hdr + 4, &len32, sizeof(len32));
        if (!sendAll(fd, hdr, sizeof(hdr))) return;
        if (bodyLen > 0) sendAll(fd, body, bodyLen);
    }

    void run() {
        if (!unixSocketPath_.empty()) {
            serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
            if (serverFd_ < 0) { std::cerr << "TestServer: unix socket failed\n"; return; }
            ::unlink(unixSocketPath_.c_str());
            sockaddr_un addr{};
            addr.sun_family = AF_UNIX;
            if (unixSocketPath_.size() >= sizeof(addr.sun_path)) {
                std::cerr << "TestServer: unix socket path too long\n";
                ::close(serverFd_);
                serverFd_ = -1;
                return;
            }
            std::strncpy(addr.sun_path, unixSocketPath_.c_str(), sizeof(addr.sun_path) - 1);
            if (bind(serverFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                std::cerr << "TestServer: unix bind failed\n";
                ::close(serverFd_);
                serverFd_ = -1;
                return;
            }
        } else {
            serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
            if (serverFd_ < 0) { std::cerr << "TestServer: socket failed\n"; return; }

            int opt = 1;
            setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(static_cast<uint16_t>(tcpPort_));

            if (bind(serverFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                std::cerr << "TestServer: bind failed on port " << tcpPort_ << "\n";
                ::close(serverFd_);
                serverFd_ = -1;
                return;
            }
        }

        listen(serverFd_, 8);

        while (running_) {
            int clientFd = accept(serverFd_, nullptr, nullptr);
            if (clientFd < 0) continue;
            handleBinaryClient(clientFd);
            shutdown(clientFd, SHUT_RDWR);
            ::close(clientFd);
        }
    }

    void handleBinaryClient(int fd) {
        uint32_t magic = 0;
        uint16_t version = 0;
        uint16_t opcode = 0;
        uint32_t bodyLen = 0;
        if (!readFull(fd, &magic, sizeof(magic))) return;
        if (!readFull(fd, &version, sizeof(version))) return;
        if (!readFull(fd, &opcode, sizeof(opcode))) return;
        if (!readFull(fd, &bodyLen, sizeof(bodyLen))) return;
        if (magic != kFluxTestMagic || version != 1) {
            std::string err = R"({"error":"bad request"})";
            sendBinaryResponse(fd, 1, 0, err.data(), err.size());
            return;
        }
        constexpr uint32_t kMaxBody = 4 * 1024 * 1024;
        if (bodyLen > kMaxBody) {
            std::string err = R"({"error":"body too large"})";
            sendBinaryResponse(fd, 1, 0, err.data(), err.size());
            return;
        }
        std::string body;
        if (bodyLen > 0) {
            body.resize(bodyLen);
            if (!readFull(fd, body.data(), bodyLen)) return;
        }

        switch (opcode) {
            case static_cast<uint16_t>(Op::GetUi):
                sendGetUi(fd);
                break;
            case static_cast<uint16_t>(Op::GetScreenshot):
                sendScreenshotBinary(fd);
                break;
            case static_cast<uint16_t>(Op::Click):
                handleClickBinary(fd, body);
                break;
            case static_cast<uint16_t>(Op::Type):
                handleTypeBinary(fd, body);
                break;
            case static_cast<uint16_t>(Op::Key):
                handleKeyBinary(fd, body);
                break;
            case static_cast<uint16_t>(Op::Scroll):
                handleScrollBinary(fd, body);
                break;
            case static_cast<uint16_t>(Op::Hover):
                handleHoverBinary(fd, body);
                break;
            case static_cast<uint16_t>(Op::Drag):
                handleDragBinary(fd, body);
                break;
            default: {
                std::string msg = R"({"error":"unknown opcode"})";
                sendBinaryResponse(fd, 1, 0, msg.data(), msg.size());
                break;
            }
        }
    }

    void sendGetUi(int fd) {
        std::string json;
        {
            std::lock_guard<std::mutex> lock(snapshotMutex_);
            json = uiTreeJSON_;
        }
        if (json.empty()) json = "{}";
        sendBinaryResponse(fd, 0, 0, json.data(), json.size());
    }

    void sendScreenshotBinary(int fd) {
        uint64_t myReq = screenshotReqSeq_.fetch_add(1, std::memory_order_acq_rel) + 1;
        window_.requestRedraw();
        wakePlatformEventLoop();

        std::unique_lock<std::mutex> lock(screenshotWaitMutex_);
        bool ok = screenshotCV_.wait_for(lock, std::chrono::seconds(5), [&]() {
            return screenshotDoneSeq_.load(std::memory_order_acquire) >= myReq;
        });
        if (!ok) {
            std::string msg = R"({"error":"screenshot timeout"})";
            sendBinaryResponse(fd, 1, 0, msg.data(), msg.size());
            return;
        }
        std::vector<uint8_t> png;
        {
            std::lock_guard<std::mutex> snapLock(snapshotMutex_);
            png = screenshot_;
        }
        if (png.empty()) {
            std::string msg = R"({"error":"no screenshot"})";
            sendBinaryResponse(fd, 1, 0, msg.data(), msg.size());
            return;
        }
        sendBinaryResponse(fd, 0, 1, png.data(), png.size());
    }

    void handleClickBinary(int fd, const std::string& body) {
        float x = 0, y = 0;
        parseFloat(body, "x", x);
        parseFloat(body, "y", y);

        enqueueAndWait({SyntheticEvent::Click, x, y, 0, 0, "", 0});

        std::string resp = R"({"ok":true,"action":"click","x":)" + std::to_string(x) + R"(,"y":)" + std::to_string(y) + "}";
        sendBinaryResponse(fd, 0, 0, resp.data(), resp.size());
    }

    void handleTypeBinary(int fd, const std::string& body) {
        std::string text = parseString(body, "text");

        enqueueAndWait({SyntheticEvent::TextInput, 0, 0, 0, 0, text, 0});

        std::string resp = R"({"ok":true,"action":"type"})";
        sendBinaryResponse(fd, 0, 0, resp.data(), resp.size());
    }

    void handleKeyBinary(int fd, const std::string& body) {
        std::string keyStr = parseString(body, "key");
        int keyCode = keyNameToCode(keyStr);

        enqueueAndWait({SyntheticEvent::KeyPress, 0, 0, 0, 0, "", keyCode});

        std::string resp = R"({"ok":true,"action":"key","key":")" + escapeJson(keyStr) + "\"}";
        sendBinaryResponse(fd, 0, 0, resp.data(), resp.size());
    }

    void handleScrollBinary(int fd, const std::string& body) {
        float x = 0, y = 0, dx = 0, dy = 0;
        parseFloat(body, "x", x);
        parseFloat(body, "y", y);
        parseFloat(body, "deltaX", dx);
        parseFloat(body, "deltaY", dy);

        enqueueAndWait({SyntheticEvent::Scroll, x, y, dx, dy, "", 0});

        std::string resp = R"({"ok":true,"action":"scroll"})";
        sendBinaryResponse(fd, 0, 0, resp.data(), resp.size());
    }

    void handleHoverBinary(int fd, const std::string& body) {
        float x = 0, y = 0;
        parseFloat(body, "x", x);
        parseFloat(body, "y", y);

        enqueueAndWait({SyntheticEvent::Hover, x, y, 0, 0, "", 0});

        std::string resp = R"({"ok":true,"action":"hover"})";
        sendBinaryResponse(fd, 0, 0, resp.data(), resp.size());
    }

    void handleDragBinary(int fd, const std::string& body) {
        float startX = 0, startY = 0, endX = 0, endY = 0;
        float steps = 10;
        parseFloat(body, "startX", startX);
        parseFloat(body, "startY", startY);
        parseFloat(body, "endX", endX);
        parseFloat(body, "endY", endY);
        parseFloat(body, "steps", steps);
        int numSteps = std::max(1, static_cast<int>(steps));

        enqueueAndWait({SyntheticEvent::MouseDown, startX, startY, 0, 0, "", 0});

        for (int i = 1; i <= numSteps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(numSteps);
            float mx = startX + (endX - startX) * t;
            float my = startY + (endY - startY) * t;
            enqueueAndWait({SyntheticEvent::Hover, mx, my, 0, 0, "", 0});
        }

        enqueueAndWait({SyntheticEvent::MouseUp, endX, endY, 0, 0, "", 0});

        std::string resp = R"({"ok":true,"action":"drag","startX":)" + std::to_string(startX)
            + R"(,"startY":)" + std::to_string(startY)
            + R"(,"endX":)" + std::to_string(endX)
            + R"(,"endY":)" + std::to_string(endY) + "}";
        sendBinaryResponse(fd, 0, 0, resp.data(), resp.size());
    }

    void enqueueAndWait(SyntheticEvent event) {
        uint64_t seq;
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            seq = ++eventSeq_;
            event.seq = seq;
            pendingEvents_.push_back(std::move(event));
        }
        window_.requestRedraw();

        wakePlatformEventLoop();

        {
            std::unique_lock<std::mutex> lock(frameMutex_);
            frameCV_.wait_for(lock, std::chrono::milliseconds(3000),
                [&]() { return renderedSeq_ >= seq; });
        }
    }

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

    Window& window_;
    int tcpPort_;
    std::string unixSocketPath_;

    std::atomic<bool> running_{false};
    std::thread thread_;
    int serverFd_ = -1;

    std::mutex snapshotMutex_;
    std::string uiTreeJSON_;
    std::vector<uint8_t> screenshot_;

    std::atomic<uint64_t> screenshotReqSeq_{0};
    std::atomic<uint64_t> screenshotDoneSeq_{0};
    std::mutex screenshotWaitMutex_;
    std::condition_variable screenshotCV_;

    std::mutex eventMutex_;
    std::vector<SyntheticEvent> pendingEvents_;
    uint64_t eventSeq_ = 0;
    uint64_t processedSeq_ = 0;

    std::mutex frameMutex_;
    std::condition_variable frameCV_;
    uint64_t frameCounter_ = 0;
    uint64_t renderedSeq_ = 0;

public:
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
