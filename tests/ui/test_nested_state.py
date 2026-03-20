"""Tests for complex state interactions: counter, derived values, badges, history."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_focus_key, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_nested_state")


class TestNestedState(unittest.TestCase):
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

    def test_initial_state(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "0")
        self.assertEqual(get_text_value(tree, "doubled:"), "0")
        self.assertEqual(get_text_value(tree, "tripled:"), "0")
        self.assertEqual(get_text_value(tree, "parity:"), "even")

    def test_increment(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-inc")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "1")
        self.assertEqual(get_text_value(tree, "doubled:"), "2")
        self.assertEqual(get_text_value(tree, "tripled:"), "3")
        self.assertEqual(get_text_value(tree, "parity:"), "odd")

    def test_increment_again(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-inc")
        self.client.click(*center_of(btn))
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "2")
        self.assertEqual(get_text_value(tree, "doubled:"), "4")
        self.assertEqual(get_text_value(tree, "parity:"), "even")

    def test_decrement(self):
        inc = find_by_focus_key(self.get_tree(), "btn-inc")
        self.client.click(*center_of(inc))

        dec = find_by_focus_key(self.get_tree(), "btn-dec")
        self.client.click(*center_of(dec))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "0")
        self.assertEqual(get_text_value(tree, "doubled:"), "0")
        self.assertEqual(get_text_value(tree, "tripled:"), "0")

    def test_decrement_to_negative(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-dec")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "-1")
        self.assertEqual(get_text_value(tree, "doubled:"), "-2")
        self.assertEqual(get_text_value(tree, "parity:"), "odd")

    def test_reset(self):
        inc = find_by_focus_key(self.get_tree(), "btn-inc")
        self.client.click(*center_of(inc))

        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-reset")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "0")
        self.assertEqual(get_text_value(tree, "doubled:"), "0")
        self.assertEqual(get_text_value(tree, "parity:"), "even")

    def test_history_records_actions(self):
        inc = find_by_focus_key(self.get_tree(), "btn-inc")
        self.client.click(*center_of(inc))
        dec = find_by_focus_key(self.get_tree(), "btn-dec")
        self.client.click(*center_of(dec))
        reset = find_by_focus_key(self.get_tree(), "btn-reset")
        self.client.click(*center_of(reset))

        tree = self.get_tree()
        history = get_text_value(tree, "history:")
        self.assertIsNotNone(history)
        self.assertGreater(len(history), 0, "History should record actions")
        self.assertIn("I", history)
        self.assertIn("D", history)
        self.assertIn("R", history)

    def test_many_increments_derived_state(self):
        inc = find_by_focus_key(self.get_tree(), "btn-inc")
        for _ in range(7):
            self.client.click(*center_of(inc))

        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "counter:"), "7")
        self.assertEqual(get_text_value(tree, "doubled:"), "14")
        self.assertEqual(get_text_value(tree, "tripled:"), "21")
        self.assertEqual(get_text_value(tree, "parity:"), "odd")


if __name__ == "__main__":
    unittest.main()
