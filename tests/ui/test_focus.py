"""Tests for focus navigation: Tab, Shift+Tab, click-to-focus, focus/blur events."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_focus_key, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_focus")

FOCUS_KEYS = ["focus-alpha", "focus-beta", "focus-gamma", "focus-delta", "focus-epsilon"]


class TestFocusNavigation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.app = FluxAppProcess(EXECUTABLE)
        cls.app.start()
        cls.client = FluxTestClient(unix_socket=cls.app.unix_socket)
        cls.client.wait_ready()
        tree = cls.client.get_ui()
        cls.initial_focus = get_text_value(tree, "focus-current:")

    @classmethod
    def tearDownClass(cls):
        cls.app.stop()

    def get_tree(self):
        return self.client.get_ui()

    def setUp(self):
        self.client.click(1, 1)

    def test_all_items_focusable(self):
        tree = self.get_tree()
        for key in FOCUS_KEYS:
            node = find_by_focus_key(tree, key)
            self.assertIsNotNone(node, f"{key} should exist")
            self.assertTrue(node.get("focusable"), f"{key} should be focusable")

    def test_initial_no_focus(self):
        self.assertEqual(self.initial_focus, "none")

    def test_tab_advances_focus(self):
        self.client.press_key("Tab")
        tree = self.get_tree()
        current = get_text_value(tree, "focus-current:")
        self.assertIsNotNone(current)
        self.assertNotEqual(current, "none", "Tab should focus something")

    def test_tab_cycles_through_items(self):
        focused_items = set()
        for _ in range(6):
            self.client.press_key("Tab")
            tree = self.get_tree()
            current = get_text_value(tree, "focus-current:")
            if current and current != "none":
                focused_items.add(current)

        self.assertGreaterEqual(len(focused_items), 3, "Tab should cycle through multiple items")

    def test_focus_count_increments(self):
        self.client.press_key("Tab")
        self.client.press_key("Tab")
        tree = self.get_tree()
        count = get_text_value(tree, "focus-count:")
        self.assertIsNotNone(count)
        self.assertGreater(int(count), 0)

    def test_blur_count_increments(self):
        self.client.press_key("Tab")
        self.client.press_key("Tab")
        self.client.press_key("Tab")
        tree = self.get_tree()
        count = get_text_value(tree, "blur-count:")
        self.assertIsNotNone(count)
        self.assertGreater(int(count), 0)

    def test_click_to_focus(self):
        tree = self.get_tree()
        target = find_by_focus_key(tree, "focus-gamma")
        self.assertIsNotNone(target)
        self.client.click(*center_of(target))
        tree = self.get_tree()
        current = get_text_value(tree, "focus-current:")
        self.assertIn("focus-gamma", current)


if __name__ == "__main__":
    unittest.main()
