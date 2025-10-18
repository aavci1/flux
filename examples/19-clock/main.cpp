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

    State hours = 10;
    State minutes = 30;
    State seconds = 0;

    window.setRootView(
        Clock {
            .hours = hours,
            .minutes = minutes,
            .seconds = seconds
        }
    );

    timeout([&hours, &minutes, &seconds]() {
        // auto now = std::chrono::system_clock::now();

        // auto dp = date::floor<date::days>(now);
        // auto time = date::make_time(now - dp);
        // hours = time.hours().count();
        // minutes = time.minutes().count();
        // seconds = time.seconds().count();
        seconds++;
    }, 1000);

    return app.exec();
}