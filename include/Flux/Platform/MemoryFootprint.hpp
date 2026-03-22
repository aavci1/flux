#pragma once

namespace flux {

/// Logs resident size and (on macOS) physical footprint via `task_info` (`phys_footprint`).
///
/// When `FLUX_MEMORY_REPORT` is set in the environment, **every** Flux app that uses
/// `Application::exec()` prints a line after the first rendered frame, tagged with the
/// executable basename (e.g. `terminal: first frame`, `llm_studio: first frame`).
/// GPU apps also log once from `GPUPlatformRenderer` immediately after GPU init
/// (`after GPU init`).
///
/// Non-macOS builds compile a no-op stub. Compare processes with `vmmap -summary <pid>`.
void logMemoryFootprintIfRequested(const char* tag);

} // namespace flux
