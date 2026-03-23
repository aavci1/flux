"""Smoke tests for the SVG example: accessibility text, framebuffer capture, and pixel sanity."""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import FluxAppProcess, FluxTestClient, find_by_text_containing
from png_pixels import read_png_rgba, region_max_channel_stats

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "svg_demo")


class TestSvgDemo(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = FluxAppProcess(EXECUTABLE)
        cls.app.start()
        cls.client = FluxTestClient(unix_socket=cls.app.unix_socket)
        cls.client.wait_ready()

    @classmethod
    def tearDownClass(cls):
        cls.app.stop()

    def test_headline_visible(self):
        tree = self.client.get_ui()
        title = find_by_text_containing(tree, "SVG component")
        self.assertGreaterEqual(
            len(title),
            1,
            "Expected main title text",
        )

    def test_screenshot_png(self):
        png = self.client.get_screenshot()
        self.assertGreater(len(png), 5000)
        self.assertEqual(png[:8], b"\x89PNG\r\n\x1a\n")

    def test_screenshot_has_visible_content_in_demo_region(self):
        """Decode framebuffer and assert the scroll area shows non-background pixels (text + SVG)."""
        png = self.client.get_screenshot()
        w, h, rgba = read_png_rgba(png)
        self.assertGreater(w, 100)
        self.assertGreater(h, 100)
        # Retina framebuffer is ~2x window; sample central band where the first SVG frames sit.
        _mn, mx, _mean = region_max_channel_stats(rgba, w, h, 80, 400, w - 80, min(h - 80, 1100))
        self.assertGreater(
            mx,
            80,
            "Framebuffer region should include bright UI (text or light SVG fills), not a flat dark bitmap",
        )


if __name__ == "__main__":
    unittest.main()
