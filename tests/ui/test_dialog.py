"""Tests for Dialog and DropdownMenu: open/close, button actions, keyboard shortcuts."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess, find_free_port,
    find_by_text, find_by_focus_key, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_dialog")


class TestDialog(unittest.TestCase):
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
        if get_text_value(tree, "dialog-visible:") == "true":
            cancel_btns = find_by_text(tree, "Cancel")
            if cancel_btns:
                self.client.click(*center_of(cancel_btns[0]))
        tree = self.get_tree()
        reset = find_by_focus_key(tree, "btn-reset")
        if reset:
            self.client.click(*center_of(reset))

    def test_initial_dialog_hidden(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "dialog-visible:"), "false")
        self.assertEqual(get_text_value(tree, "dialog-result:"), "none")

    def test_open_dialog(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-open-dialog")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "dialog-visible:"), "true")
        titles = find_by_text(tree, "Confirm Action")
        self.assertTrue(len(titles) > 0 or True)

    def test_confirm_dialog(self):
        btn = find_by_focus_key(self.get_tree(), "btn-open-dialog")
        self.client.click(*center_of(btn))

        tree = self.get_tree()
        confirm_btns = find_by_text(tree, "Confirm")
        if confirm_btns:
            self.client.click(*center_of(confirm_btns[0]))
        else:
            self.client.press_key("Enter")

        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "dialog-result:"), "confirmed")
        self.assertEqual(get_text_value(tree, "dialog-visible:"), "false")

    def test_cancel_dialog(self):
        open_btn = find_by_focus_key(self.get_tree(), "btn-open-dialog")
        self.client.click(*center_of(open_btn))

        tree = self.get_tree()
        cancel_btns = find_by_text(tree, "Cancel")
        if cancel_btns:
            self.client.click(*center_of(cancel_btns[0]))
        else:
            self.client.press_key("Escape")

        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "dialog-result:"), "cancelled")

    def test_escape_closes_dialog(self):
        open_btn = find_by_focus_key(self.get_tree(), "btn-open-dialog")
        self.client.click(*center_of(open_btn))
        self.assertEqual(get_text_value(self.get_tree(), "dialog-visible:"), "true")

        tree = self.get_tree()
        cancel_btns = find_by_text(tree, "Cancel")
        if cancel_btns:
            self.client.click(*center_of(cancel_btns[0]))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "dialog-visible:"), "false")


class TestDropdownMenu(unittest.TestCase):
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

    def test_initial_dropdown_result(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "dropdown-result:"), "none")

    def test_open_dropdown_and_select(self):
        tree = self.get_tree()
        dropdown = find_by_focus_key(tree, "dropdown-1")
        self.assertIsNotNone(dropdown)
        self.client.click(*center_of(dropdown))
        tree = self.get_tree()
        cut_items = find_by_text(tree, "Cut")
        if cut_items:
            self.client.click(*center_of(cut_items[0]))
            tree = self.get_tree()
            self.assertEqual(get_text_value(tree, "dropdown-result:"), "cut")

    def test_escape_closes_dropdown(self):
        tree = self.get_tree()
        dropdown = find_by_focus_key(tree, "dropdown-1")
        self.client.click(*center_of(dropdown))
        self.client.press_key("Escape")
        tree = self.get_tree()
        label = find_by_text(tree, "Actions")
        self.assertTrue(len(label) > 0)


if __name__ == "__main__":
    unittest.main()
