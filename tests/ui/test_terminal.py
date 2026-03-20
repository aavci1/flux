"""
UI tests for the Flux Terminal example (PTY + TerminalView).

Requires a Unix-like host (same as the terminal executable).
"""

import os
import time
import unittest
from typing import Optional

from flux_test_client import FluxAppProcess, FluxTestClient, center_of, find_by_focus_key


def _terminal_exe(build_dir: str) -> str:
    return os.path.join(build_dir, "ui_test_terminal")


def _wait_for_terminal_view(client: FluxTestClient, timeout: float = 25.0) -> None:
    """Root may be a leaf view (no `children`); wait_ready() would never succeed."""
    deadline = time.time() + timeout
    last_err: Optional[Exception] = None
    while time.time() < deadline:
        try:
            tree = client.get_ui()
            if find_by_focus_key(tree, "terminal") is not None:
                return
        except (OSError, ConnectionError, RuntimeError, ValueError) as e:
            last_err = e
        time.sleep(0.25)
    loc = client.unix_socket or f"port {client.port}"
    msg = f"terminal view not in UI tree ({loc}) within {timeout}s"
    if last_err:
        msg += f": {last_err}"
    raise TimeoutError(msg)


class TestTerminal(unittest.TestCase):
    def test_font_zoom_reports_in_ui_tree(self):
        build_dir = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
        build_dir = os.path.abspath(build_dir)
        exe = _terminal_exe(build_dir)
        self.assertTrue(os.path.isfile(exe), f"missing {exe}; build with BUILD_UI_TESTS=ON")

        proc = FluxAppProcess(exe)
        proc.start()
        try:
            client = FluxTestClient(unix_socket=proc.unix_socket)
            _wait_for_terminal_view(client)
            tree = client.get_ui()
            term = find_by_focus_key(tree, "terminal")
            self.assertIsNotNone(term, "terminal view should expose focusKey=terminal")
            cx, cy = center_of(term)
            client.click(cx, cy)

            tree = client.get_ui()
            term = find_by_focus_key(tree, "terminal")
            v0 = float(term.get("value", "14"))
            client.press_key("Equal", modifiers=["ctrl"])
            tree = client.get_ui()
            term = find_by_focus_key(tree, "terminal")
            v1 = float(term.get("value", str(v0)))
            self.assertGreater(v1, v0, "Ctrl++ should increase fontSize (accessible value)")

            client.press_key("Minus", modifiers=["ctrl"])
            tree = client.get_ui()
            term = find_by_focus_key(tree, "terminal")
            v2 = float(term.get("value", str(v1)))
            self.assertLess(v2, v1, "Ctrl+- should decrease font size")

            client.press_key("0", modifiers=["ctrl"])
            tree = client.get_ui()
            term = find_by_focus_key(tree, "terminal")
            v3 = float(term.get("value", str(v2)))
            self.assertAlmostEqual(v3, 14.0, places=3, msg="Ctrl+0 should reset to default 14px")
        finally:
            proc.stop()


if __name__ == "__main__":
    unittest.main()
