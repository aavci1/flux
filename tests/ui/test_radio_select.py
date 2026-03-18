"""Tests for RadioButton groups and SelectInput."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess, find_free_port,
    find_by_focus_key, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_radio_select")


class TestRadioButton(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = find_free_port()
        cls.app = FluxAppProcess(EXECUTABLE, port=cls.port)
        cls.app.start()
        cls.client = FluxTestClient(port=cls.port)
        cls.client.wait_ready()

    @classmethod
    def tearDownClass(cls):
        cls.app.stop()

    def get_tree(self):
        return self.client.get_ui()

    def setUp(self):
        tree = self.get_tree()
        radio1 = find_by_focus_key(tree, "radio-1")
        if radio1:
            self.client.click(*center_of(radio1))

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


class TestSelectInput(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = find_free_port()
        cls.app = FluxAppProcess(EXECUTABLE, port=cls.port)
        cls.app.start()
        cls.client = FluxTestClient(port=cls.port)
        cls.client.wait_ready()

    @classmethod
    def tearDownClass(cls):
        cls.app.stop()

    def get_tree(self):
        return self.client.get_ui()

    def setUp(self):
        tree = self.get_tree()
        select = find_by_focus_key(tree, "select-1")
        self.client.click(*center_of(select))
        self.client.press_key("Escape")
        for _ in range(10):
            self.client.press_key("Up")

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
