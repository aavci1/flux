"""Tests for RadioButton groups and SelectInput."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_focus_key, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_radio_select")


def _reset_radio_and_select(client):
    tree = client.get_ui()
    radio1 = find_by_focus_key(tree, "radio-1")
    if radio1:
        client.click(*center_of(radio1))
    tree = client.get_ui()
    select = find_by_focus_key(tree, "select-1")
    client.click(*center_of(select))
    client.press_key("Escape")
    for _ in range(10):
        client.press_key("Up")


class TestRadioSelectApp(unittest.TestCase):
    """Single app; `setUp` selects first radio and resets select to index 0."""

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
        _reset_radio_and_select(self.client)

    # --- radio ---
    def test_initial_selection(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "radio-selected:"), "opt1")

    def test_click_selects_option(self):
        tree = self.get_tree()
        radio2 = find_by_focus_key(tree, "radio-2")
        self.assertIsNotNone(radio2)
        self.client.click(*center_of(radio2))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "radio-selected:"), "opt2")

    def test_click_different_option(self):
        tree = self.get_tree()
        radio3 = find_by_focus_key(tree, "radio-3")
        self.client.click(*center_of(radio3))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "radio-selected:"), "opt3")

    def test_reselect_first(self):
        tree = self.get_tree()
        radio1 = find_by_focus_key(tree, "radio-1")
        self.client.click(*center_of(radio1))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "radio-selected:"), "opt1")

    # --- select ---
    def test_initial_select_state(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "select-index:"), "0")
        self.assertEqual(get_text_value(tree, "select-label:"), "Apple")

    def test_keyboard_navigation(self):
        self.client.press_key("Down")
        tree = self.get_tree()
        idx = get_text_value(tree, "select-index:")
        self.assertEqual(idx, "1")

    def test_keyboard_arrow_up(self):
        self.client.press_key("Up")
        tree = self.get_tree()
        idx = get_text_value(tree, "select-index:")
        self.assertEqual(idx, "0")


if __name__ == "__main__":
    unittest.main()
