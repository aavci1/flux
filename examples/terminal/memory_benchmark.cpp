#include "TerminalEmulator.hpp"

#include <mach/mach.h>
#include <cstdio>
#include <chrono>
#include <string>

static uint64_t physFootprintKB() {
    task_vm_info_data_t info{};
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    kern_return_t kr = task_info(mach_task_self(), TASK_VM_INFO,
                                reinterpret_cast<task_info_t>(&info), &count);
    if (kr != KERN_SUCCESS) return 0;
    return static_cast<uint64_t>(info.phys_footprint) / 1024;
}

static double mb(uint64_t kb) { return static_cast<double>(kb) / 1024.0; }

static std::string generateLine(int cols) {
    std::string line;
    line.reserve(static_cast<size_t>(cols) * 4);
    for (int c = 0; c < cols; ++c) {
        char ch = 'A' + (c % 26);
        line += "\033[3";
        line += static_cast<char>('1' + (c % 7));
        line += 'm';
        line += ch;
    }
    line += "\033[0m\r\n";
    return line;
}

int main() {
    printf("=== Flux Terminal Memory Benchmark ===\n\n");
    printf("sizeof(Cell)          = %zu\n", sizeof(flux::term::Cell));
    printf("sizeof(std::string)   = %zu\n", sizeof(std::string));
    printf("sizeof(Color)         = %zu\n", sizeof(flux::Color));

    constexpr int COLS = 120;
    constexpr int ROWS = 40;
    uint64_t base = physFootprintKB();
    printf("\nBaseline: %.2f MB\n", mb(base));

    auto emu = std::make_unique<flux::term::TerminalEmulator>(COLS, ROWS);

    for (int target : {500, 2000, 5000, 10000}) {
        emu = std::make_unique<flux::term::TerminalEmulator>(COLS, ROWS);
        uint64_t pre = physFootprintKB();

        std::string bulk;
        for (int r = 0; r < target; ++r) bulk += generateLine(COLS);
        emu->feed(bulk.data(), bulk.size());
        bulk.clear(); bulk.shrink_to_fit();

        uint64_t afterFeed = physFootprintKB();

        {
            auto snap = emu->snapshot();
            uint64_t afterSnap = physFootprintKB();

            size_t cells = 0;
            for (const auto& l : snap.lines) cells += l.size();

            printf("\n--- %5d lines ---\n", target);
            printf("  Buffer:   +%.2f MB\n", mb(afterFeed - pre));
            printf("  Snapshot: +%.2f MB  (%zu cells, %.2f MB estimated)\n",
                   mb(afterSnap - afterFeed), cells,
                   static_cast<double>(cells * sizeof(flux::term::Cell)) / (1024.0 * 1024.0));
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; ++i) { auto s = emu->snapshot(); (void)s; }
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        printf("  100x snapshot(): %.1f ms (%.2f ms/frame)\n", ms, ms / 100.0);

        {
            uint64_t preRange = physFootprintKB();
            auto sr = emu->snapshotRange(emu->totalLines() - ROWS, ROWS);
            uint64_t postRange = physFootprintKB();
            printf("  snapshotRange(%d visible): +%.2f MB  (%zu lines copied)\n",
                   ROWS, mb(postRange - preRange), sr.lines.size());
        }

        t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 100; ++i) {
            auto s = emu->snapshotRange(emu->totalLines() - ROWS, ROWS);
            (void)s;
        }
        t1 = std::chrono::high_resolution_clock::now();
        ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        printf("  100x snapshotRange(): %.1f ms (%.2f ms/frame)\n", ms, ms / 100.0);
    }

    printf("\nFinal footprint: %.2f MB (delta: +%.2f MB)\n",
           mb(physFootprintKB()), mb(physFootprintKB() - base));
    return 0;
}
