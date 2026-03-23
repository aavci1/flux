#!/usr/bin/env python3
"""
Capture svg_demo in --test-mode and write a PNG next to the example sources for visual inspection.

  FLUX_BUILD_DIR=build python3 tests/ui/save_svg_demo_screenshot.py

Output: examples/svg-demo/verify-screenshot.png
"""

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
OUT_PATH = os.path.join(PROJECT_ROOT, "examples", "svg-demo", "verify-screenshot.png")

sys.path.insert(0, SCRIPT_DIR)
from flux_test_client import FluxAppProcess, FluxTestClient  # noqa: E402


def main() -> int:
    build_dir = os.environ.get("FLUX_BUILD_DIR", os.path.join(PROJECT_ROOT, "build"))
    exe = os.path.join(build_dir, "svg_demo")
    if not os.path.isfile(exe):
        print(f"[error] Missing {exe} — build svg_demo first.", file=sys.stderr)
        return 1

    app = FluxAppProcess(exe)
    app.start()
    try:
        client = FluxTestClient(unix_socket=app.unix_socket)
        client.wait_ready()
        png = client.get_screenshot()
    finally:
        app.stop()

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    with open(OUT_PATH, "wb") as f:
        f.write(png)
    print(f"Wrote {OUT_PATH} ({len(png)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
