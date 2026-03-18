"""Tests for visibility toggling, opacity changes, and layout recalculation."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess, find_free_port,
    find_by_text, find_by_focus_key, get_bounds, center_of, get_text_value,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_visibility")


class TestVisibility(unittest.TestCase):
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
        if get_text_value(tree, "box-visible:") != "true":
            btn = find_by_focus_key(tree, "btn-toggle-vis")
            self.client.click(*center_of(btn))

    def test_initial_visible(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "box-visible:"), "true")
        box = find_by_text(tree, "visible-box-content")
        self.assertTrue(len(box) > 0, "Box should be visible initially")

    def test_toggle_hides_box(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-toggle-vis")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "box-visible:"), "false")

    def test_layout_shifts_when_hidden(self):
        # Get marker position when box is hidden
        tree = self.get_tree()
        if get_text_value(tree, "box-visible:") == "true":
            btn = find_by_focus_key(tree, "btn-toggle-vis")
            self.client.click(*center_of(btn))
        tree = self.get_tree()
        marker_hidden = find_by_text(tree, "below-box-marker")
        self.assertTrue(len(marker_hidden) > 0)
        y_hidden = get_bounds(marker_hidden[0])["y"]

        # Show the box again
        btn = find_by_focus_key(tree, "btn-toggle-vis")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        marker_visible = find_by_text(tree, "below-box-marker")
        y_visible = get_bounds(marker_visible[0])["y"]

        self.assertGreater(y_visible, y_hidden, "Marker should be lower when box is visible")

    def test_toggle_count_increments(self):
        tree = self.get_tree()
        count_before = int(get_text_value(tree, "toggle-count:") or "0")
        btn = find_by_focus_key(tree, "btn-toggle-vis")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        count_after = int(get_text_value(tree, "toggle-count:") or "0")
        self.assertEqual(count_after, count_before + 1)


class TestOpacity(unittest.TestCase):
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
        if get_text_value(tree, "box-opacity:") != "1.0":
            btn = find_by_focus_key(tree, "btn-opacity-full")
            self.client.click(*center_of(btn))

    def test_initial_opacity_full(self):
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "box-opacity:"), "1.0")

    def test_set_opacity_half(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-opacity-half")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "box-opacity:"), "0.5")

    def test_restore_opacity_full(self):
        tree = self.get_tree()
        btn = find_by_focus_key(tree, "btn-opacity-full")
        self.client.click(*center_of(btn))
        tree = self.get_tree()
        self.assertEqual(get_text_value(tree, "box-opacity:"), "1.0")


if __name__ == "__main__":
    unittest.main()
