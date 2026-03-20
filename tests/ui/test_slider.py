"""Tests for Slider: drag, arrow keys, Home/End, value constraints."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_focus_key, get_bounds, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_slider")

_STEPPED = frozenset({
    "test_initial_stepped_value",
    "test_right_increments_by_step",
    "test_stepped_home_sets_min",
    "test_stepped_end_sets_max",
})


class TestSliderApp(unittest.TestCase):
    """Single app; `setUp` focuses the slider under test and sends Home."""

    @classmethod
    def setUpClass(cls):
        cls.app = FluxAppProcess(EXECUTABLE)
        cls.app.start()
        cls.client = FluxTestClient(unix_socket=cls.app.unix_socket)
        cls.client.wait_ready()

    @classmethod
    def tearDownClass(cls):
        cls.app.stop()

    def get_tree(self):
        return self.client.get_ui()

    def setUp(self):
        m = self._testMethodName
        tree = self.client.get_ui()
        if m in _STEPPED:
            key = "slider-stepped"
        else:
            key = "slider-continuous"
        slider = find_by_focus_key(tree, key)
        self.client.click(*center_of(slider))
        self.client.press_key("Home")

    # --- continuous ---
    def test_initial_value(self):
        tree = self.get_tree()
        val = get_text_value(tree, "slider-value:")
        self.assertIsNotNone(val)
        self.assertAlmostEqual(float(val), 0.0, delta=0.01)

    def test_keyboard_right_increases(self):
        val_before = float(get_text_value(self.get_tree(), "slider-value:"))
        self.client.press_key("Right")
        val_after = float(get_text_value(self.get_tree(), "slider-value:"))
        self.assertGreater(val_after, val_before)

    def test_keyboard_left_decreases(self):
        self.client.press_key("Right")
        self.client.press_key("Right")
        val_before = float(get_text_value(self.get_tree(), "slider-value:"))
        self.client.press_key("Left")
        val_after = float(get_text_value(self.get_tree(), "slider-value:"))
        self.assertLess(val_after, val_before)

    def test_continuous_home_sets_min(self):
        self.client.press_key("Right")
        self.client.press_key("Right")
        self.client.press_key("Home")
        tree = self.get_tree()
        val = float(get_text_value(tree, "slider-value:"))
        self.assertAlmostEqual(val, 0.0, delta=0.01)

    def test_continuous_end_sets_max(self):
        self.client.press_key("End")
        tree = self.get_tree()
        val = float(get_text_value(tree, "slider-value:"))
        self.assertAlmostEqual(val, 1.0, delta=0.01)

    def test_drag_changes_value(self):
        tree = self.get_tree()
        slider = find_by_focus_key(tree, "slider-continuous")
        b = get_bounds(slider)
        start_x = b["x"] + b["w"] * 0.2
        end_x = b["x"] + b["w"] * 0.8
        y = b["y"] + b["h"] / 2

        self.client.drag(start_x, y, end_x, y, steps=5)
        tree = self.get_tree()
        val = float(get_text_value(tree, "slider-value:"))
        self.assertGreater(val, 0.3, "Drag should change slider value")

    # --- stepped ---
    def test_initial_stepped_value(self):
        tree = self.get_tree()
        val = get_text_value(tree, "stepped-value:")
        self.assertIsNotNone(val)
        self.assertAlmostEqual(float(val), 0.0, delta=1)

    def test_right_increments_by_step(self):
        val_before = float(get_text_value(self.get_tree(), "stepped-value:"))
        self.client.press_key("Right")
        val_after = float(get_text_value(self.get_tree(), "stepped-value:"))
        self.assertAlmostEqual(val_after - val_before, 10.0, delta=1)

    def test_stepped_home_sets_min(self):
        self.client.press_key("Right")
        self.client.press_key("Right")
        self.client.press_key("Home")
        tree = self.get_tree()
        val = float(get_text_value(tree, "stepped-value:"))
        self.assertAlmostEqual(val, 0.0, delta=1)

    def test_stepped_end_sets_max(self):
        self.client.press_key("End")
        tree = self.get_tree()
        val = float(get_text_value(tree, "stepped-value:"))
        self.assertAlmostEqual(val, 100.0, delta=1)


if __name__ == "__main__":
    unittest.main()
