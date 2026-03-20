"""Tests for Button interactivity: click, counter, keyboard activation, focus."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_text, find_by_focus_key, get_bounds, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_button")


class TestButton(unittest.TestCase):
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
        tree = self.get_tree()
        reset_btn = find_by_focus_key(tree, "btn-reset")
        if reset_btn:
            self.client.click(*center_of(reset_btn))

    def test_initial_counts_zero(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "count-a:"), "0")
        self.assertEqual(get_text_value(tree, "count-b:"), "0")
        self.assertEqual(get_text_value(tree, "count-c:"), "0")
        self.assertEqual(get_text_value(tree, "last-activated:"), "none")

    def test_click_button_a(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-a")
        self.assertIsNotNone(btn)
        cx, cy = center_of(btn)
        self.client.click(cx, cy)
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "count-a:"), "1")
        self.assertEqual(get_text_value(tree, "last-activated:"), "A")

    def test_click_button_b(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-b")
        cx, cy = center_of(btn)
        self.client.click(cx, cy)
        tree = self.get_tree()
        val = get_text_value(tree, "count-b:")
        self.assertIsNotNone(val)
        self.assertGreaterEqual(int(val), 1)

    def test_multiple_clicks_increment(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-c")
        cx, cy = center_of(btn)
        for _ in range(3):
            self.client.click(cx, cy)
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "count-c:"), "3")

    def test_reset_clears_all(self):
        tree = self.get_tree()
        # Click something first
        btn_a = find_by_focus_key(tree, "btn-a")
        self.client.click(*center_of(btn_a))
        # Reset
        reset_btn = find_by_focus_key(self.get_tree(), "btn-reset")
        self.client.click(*center_of(reset_btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "count-a:"), "0")
        self.assertEqual(get_text_value(tree, "count-b:"), "0")
        self.assertEqual(get_text_value(tree, "count-c:"), "0")
        self.assertEqual(get_text_value(tree, "last-activated:"), "none")

    def test_buttons_are_focusable(self):
        tree = self.get_tree()
        for key in ["btn-a", "btn-b", "btn-c"]:
            node = find_by_focus_key(tree, key)
            self.assertIsNotNone(node, f"{key} should exist")
            self.assertTrue(node.get("focusable"), f"{key} should be focusable")

    def test_keyboard_activation_via_tab_and_enter(self):
        # Tab to first button and press Enter
        self.client.press_key("Tab")
        self.client.press_key("Enter")
        tree = self.get_tree()
        # At least one button should have been activated
        total = (
            int(get_text_value(tree, "count-a:") or "0") +
            int(get_text_value(tree, "count-b:") or "0") +
            int(get_text_value(tree, "count-c:") or "0")
        )
        self.assertGreaterEqual(total, 1, "Tab+Enter should activate a button")


if __name__ == "__main__":
    unittest.main()
