#include <Flux.hpp>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>

#include "./clock.hpp"

using namespace flux;

void timeout(std::function<void()> func, int interval) {
    std::thread([func, interval]()
    {
      while (true)
      {
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
        func();
        std::this_thread::sleep_until(x);
      }
    }).detach();
}


int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {800, 800},
        .title = "Clock"
    });

    Property hours = 10;
    Property minutes = 30;
    Property seconds = 0;

    window.setRootView(
        Clock {
            .hours = hours,
            .minutes = minutes,
            .seconds = seconds
        }
    );

    timeout([&hours, &minutes, &seconds]() {
        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        // Convert to local time
        std::tm* local_time = std::localtime(&now_time);

        // Extract hour, minute, second as integers
        hours = local_time->tm_hour;     // 0-23
        minutes = local_time->tm_min;    // 0-59
        seconds = local_time->tm_sec;    // 0-59
    }, 1000);

    return app.exec();
}