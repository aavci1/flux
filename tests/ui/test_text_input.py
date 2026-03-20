"""Tests for TextInput and TextArea: typing, editing, placeholder, password, max length."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_focus_key, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_text_input")


def _reset_all(client, tree):
    """Click the reset button to clear all inputs."""
    reset_btn = find_by_focus_key(tree, "btn-reset")
    if reset_btn:
        client.click(*center_of(reset_btn))


class TestTextInputApp(unittest.TestCase):
    """Single app; `setUp` resets all inputs before each test."""

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
        _reset_all(self.client, self.get_tree())

    # --- basic ---
    def test_initial_empty(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "input-value:"), "")
        self.assertEqual(get_text_value(tree, "input-length:"), "0")

    def test_type_text(self):
        tree = self.get_tree()
        inp = find_by_focus_key(tree, "input-basic")
        self.assertIsNotNone(inp)
        self.client.click(*center_of(inp))
        self.client.type_text("hello")
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "input-value:"), "hello")
        self.assertEqual(get_text_value(tree, "input-length:"), "5")

    def test_backspace(self):
        tree = self.get_tree()
        inp = find_by_focus_key(tree, "input-basic")
        self.client.click(*center_of(inp))
        self.client.type_text("hello")
        self.client.press_key("Backspace")
        self.client.press_key("Backspace")
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "input-value:"), "hel")

    def test_type_more(self):
        tree = self.get_tree()
        inp = find_by_focus_key(tree, "input-basic")
        self.client.click(*center_of(inp))
        self.client.type_text("hel")
        self.client.type_text("p")
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "input-value:"), "help")

    def test_enter_fires_return(self):
        tree = self.get_tree()
        inp = find_by_focus_key(tree, "input-basic")
        self.client.click(*center_of(inp))
        self.client.press_key("Enter")
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "return-status:"), "return-pressed")

    # --- password ---
    def test_password_typing(self):
        tree = self.get_tree()
        inp = find_by_focus_key(tree, "input-password")
        self.assertIsNotNone(inp)
        self.client.click(*center_of(inp))
        self.client.type_text("secret")
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "password-length:"), "6")

    # --- max length ---
    def test_max_length_enforcement(self):
        tree = self.get_tree()
        inp = find_by_focus_key(tree, "input-limited")
        self.assertIsNotNone(inp)
        self.client.click(*center_of(inp))
        self.client.type_text("abcdefgh")
        tree = self.get_tree()
        val = get_text_value(tree, "limited-value:")
        self.assertIsNotNone(val)
        self.assertLessEqual(len(val), 5, "Max length 5 should be enforced")

    # --- textarea ---
    def test_textarea_typing(self):
        tree = self.get_tree()
        area = find_by_focus_key(tree, "input-area")
        self.assertIsNotNone(area)
        self.client.click(*center_of(area))
        self.client.type_text("line one")
        tree = self.get_tree()
        val = get_text_value(tree, "area-value:")
        self.assertEqual(val, "line one")

    def test_textarea_newline(self):
        tree = self.get_tree()
        area = find_by_focus_key(tree, "input-area")
        self.client.click(*center_of(area))
        self.client.type_text("line one")
        self.client.press_key("Enter")
        self.client.type_text("line two")
        tree = self.get_tree()
        val = get_text_value(tree, "area-value:")
        self.assertIn("line two", val)


if __name__ == "__main__":
    unittest.main()
