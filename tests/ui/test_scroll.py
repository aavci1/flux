"""Tests for ScrollArea: scroll events, content visibility."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_text, get_bounds, center_of, find_by_type,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_scroll")


class TestScrollArea(unittest.TestCase):
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

    def test_scroll_items_exist(self):
        tree = self.get_tree()
        item0 = find_by_text(tree, "scroll-item-0")
        self.assertTrue(len(item0) > 0, "First scroll item should exist")
        item5 = find_by_text(tree, "scroll-item-5")
        self.assertTrue(len(item5) > 0, "Item 5 should exist")

    def test_items_in_vertical_order(self):
        tree = self.get_tree()
        items = []
        for i in range(5):
            nodes = find_by_text(tree, f"scroll-item-{i}")
            if nodes:
                items.append((i, get_bounds(nodes[0])["y"]))
        for i in range(len(items) - 1):
            self.assertLess(items[i][1], items[i + 1][1])

    def test_scroll_down_shifts_items(self):
        tree = self.get_tree()
        item0_before = find_by_text(tree, "scroll-item-0")
        y_before = get_bounds(item0_before[0])["y"]

        # Find a scroll area to scroll within
        scroll_areas = find_by_type(tree, "ScrollArea")
        if scroll_areas:
            cx, cy = center_of(scroll_areas[0])
        else:
            cx, cy = 250, 300

        self.client.scroll(cx, cy, 0, 50)
        tree = self.get_tree()
        item0_after = find_by_text(tree, "scroll-item-0")
        if item0_after:
            y_after = get_bounds(item0_after[0])["y"]
            self.assertLess(y_after, y_before, "Scrolling down should move items up")

    def test_many_items_present(self):
        tree = self.get_tree()
        count = 0
        for i in range(40):
            if find_by_text(tree, f"scroll-item-{i}"):
                count += 1
        self.assertGreater(count, 10, "Many scroll items should be in the tree")


if __name__ == "__main__":
    unittest.main()
