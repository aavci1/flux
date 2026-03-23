#!/usr/bin/env python3
"""
Flux UI Test Runner

Builds the UI test apps (if needed) and runs all Python test suites against them.
Each test suite launches its own Flux app subprocess in --test-mode with a Unix
domain socket for binary test IPC (see flux_test_client.py).

Usage:
    python3 tests/ui/run_tests.py                  # run all UI tests
    python3 tests/ui/run_tests.py test_button       # run only test_button
    python3 tests/ui/run_tests.py --build           # force rebuild before testing
    python3 tests/ui/run_tests.py --list            # list available test suites
    python3 tests/ui/run_tests.py -v                # verbose output
"""

import argparse
import os
import subprocess
import sys
import unittest

_IS_UNIX = sys.platform != "win32"

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
DEFAULT_BUILD_DIR = os.path.join(PROJECT_ROOT, "build")

ALL_TEST_MODULES = [
    "test_layout",
    "test_button",
    "test_checkbox_toggle",
    "test_radio_select",
    "test_text_input",
    "test_focus",
    "test_scroll",
    "test_visibility",
    "test_slider",
    "test_dialog",
    "test_nested_state",
    "test_svg_demo",
]
if _IS_UNIX:
    ALL_TEST_MODULES.append("test_terminal")

# Maps unittest module name -> CMake executable name (default: "ui_" + module).
UI_TEST_EXECUTABLE_OVERRIDES = {
    "test_svg_demo": "svg_demo",
}


def executable_for_module(module: str) -> str:
    return UI_TEST_EXECUTABLE_OVERRIDES.get(module, "ui_" + module)


UI_TEST_TARGETS = [
    "ui_test_layout",
    "ui_test_button",
    "ui_test_checkbox_toggle",
    "ui_test_radio_select",
    "ui_test_text_input",
    "ui_test_focus",
    "ui_test_scroll",
    "ui_test_visibility",
    "ui_test_slider",
    "ui_test_dialog",
    "ui_test_nested_state",
]
if _IS_UNIX:
    UI_TEST_TARGETS.append("ui_test_terminal")


def build_ui_tests(build_dir: str):
    """Configure and build the UI test targets."""
    os.makedirs(build_dir, exist_ok=True)

    print(f"[build] Configuring CMake in {build_dir} ...")
    result = subprocess.run(
        ["cmake", PROJECT_ROOT, "-DBUILD_UI_TESTS=ON", "-DBUILD_EXAMPLES=ON"],
        cwd=build_dir,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)

    print(f"[build] Building UI test targets ...")
    build_targets = UI_TEST_TARGETS + ["svg_demo"]
    result = subprocess.run(
        ["cmake", "--build", ".", "--target"] + build_targets + ["--parallel"],
        cwd=build_dir,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        sys.exit(1)

    print("[build] Build succeeded.\n")


def check_executables(build_dir: str, modules: list[str]) -> bool:
    """Check that all required test executables exist."""
    missing = []
    for module in modules:
        target = executable_for_module(module)
        exe_path = os.path.join(build_dir, target)
        if not os.path.isfile(exe_path):
            missing.append(target)
    if missing:
        print(f"[error] Missing executables: {', '.join(missing)}")
        print(f"[error] Run with --build or build with: cmake --build {build_dir} --target {' '.join(missing)}")
        return False
    return True


def run_tests(modules: list[str], build_dir: str, verbosity: int = 1) -> bool:
    os.environ["FLUX_BUILD_DIR"] = build_dir

    sys.path.insert(0, SCRIPT_DIR)

    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    for module_name in modules:
        try:
            module = __import__(module_name)
            suite.addTests(loader.loadTestsFromModule(module))
        except ImportError as e:
            print(f"[error] Failed to import {module_name}: {e}")
            return False

    runner = unittest.TextTestRunner(verbosity=verbosity)
    result = runner.run(suite)
    return result.wasSuccessful()


def main():
    parser = argparse.ArgumentParser(description="Flux UI Test Runner")
    parser.add_argument("modules", nargs="*", help="Specific test modules to run (default: all)")
    parser.add_argument("--build", action="store_true", help="Build UI test targets before running")
    parser.add_argument("--build-dir", default=DEFAULT_BUILD_DIR, help="CMake build directory")
    parser.add_argument("--list", action="store_true", help="List available test modules")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose test output")
    args = parser.parse_args()

    if args.list:
        print("Available UI test modules:")
        for m in ALL_TEST_MODULES:
            print(f"  {m}")
        sys.exit(0)

    modules = args.modules if args.modules else ALL_TEST_MODULES

    for m in modules:
        if m not in ALL_TEST_MODULES:
            print(f"[error] Unknown test module: {m}")
            print(f"[error] Available: {', '.join(ALL_TEST_MODULES)}")
            sys.exit(1)

    build_dir = os.path.abspath(args.build_dir)

    if args.build:
        build_ui_tests(build_dir)

    if not check_executables(build_dir, modules):
        sys.exit(1)

    verbosity = 2 if args.verbose else 1
    success = run_tests(modules, build_dir, verbosity)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
