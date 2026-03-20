"""Tests for the layout system: VStack, HStack, Grid, spacing, padding, justify, align, expansion."""

import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from flux_test_client import (
    FluxTestClient, FluxAppProcess,
    find_by_text, find_by_text_containing, get_bounds, center_of,
)

BUILD_DIR = os.environ.get("FLUX_BUILD_DIR", os.path.join(os.path.dirname(__file__), "..", "..", "build"))
EXECUTABLE = os.path.join(BUILD_DIR, "ui_test_layout")


class TestLayout(unittest.TestCase):
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

    # --- VStack ---

    def test_vstack_children_exist(self):
        tree = self.get_tree()
        for i in range(3):
            nodes = find_by_text(tree, f"vstack-child-{i}")
            self.assertTrue(len(nodes) > 0, f"vstack-child-{i} not found")

    def test_vstack_vertical_ordering(self):
        tree = self.get_tree()
        children = [find_by_text(tree, f"vstack-child-{i}")[0] for i in range(3)]
        for i in range(len(children) - 1):
            y_curr = get_bounds(children[i])["y"]
            y_next = get_bounds(children[i + 1])["y"]
            self.assertLess(y_curr, y_next, f"vstack-child-{i} should be above vstack-child-{i+1}")

    # --- HStack ---

    def test_hstack_children_exist(self):
        tree = self.get_tree()
        for i in range(3):
            nodes = find_by_text(tree, f"hstack-child-{i}")
            self.assertTrue(len(nodes) > 0, f"hstack-child-{i} not found")

    def test_hstack_horizontal_ordering(self):
        tree = self.get_tree()
        children = [find_by_text(tree, f"hstack-child-{i}")[0] for i in range(3)]
        for i in range(len(children) - 1):
            x_curr = get_bounds(children[i])["x"]
            x_next = get_bounds(children[i + 1])["x"]
            self.assertLess(x_curr, x_next, f"hstack-child-{i} should be left of hstack-child-{i+1}")

    # --- Grid 2x2 ---

    def test_grid_2x2_cells_exist(self):
        tree = self.get_tree()
        for i in range(4):
            nodes = find_by_text(tree, f"grid-cell-{i}")
            self.assertTrue(len(nodes) > 0, f"grid-cell-{i} not found")

    def test_grid_2x2_layout(self):
        tree = self.get_tree()
        cells = [find_by_text(tree, f"grid-cell-{i}")[0] for i in range(4)]
        b = [get_bounds(c) for c in cells]
        # Row 0: cells 0,1 should share roughly the same y
        self.assertAlmostEqual(b[0]["y"], b[1]["y"], delta=5)
        # Row 1: cells 2,3 should share roughly the same y
        self.assertAlmostEqual(b[2]["y"], b[3]["y"], delta=5)
        # Column ordering: cell 0 left of cell 1
        self.assertLess(b[0]["x"], b[1]["x"])
        # Row ordering: cell 0 above cell 2
        self.assertLess(b[0]["y"], b[2]["y"])

    # --- JustifyContent ---

    def test_justify_center(self):
        tree = self.get_tree()
        item = find_by_text(tree, "justify-center-item")
        self.assertTrue(len(item) > 0)
        container = find_by_text(tree, "section-justify")
        self.assertTrue(len(container) > 0)

    def test_justify_end(self):
        tree = self.get_tree()
        nodes = find_by_text(tree, "justify-end-item")
        self.assertTrue(len(nodes) > 0)

    def test_justify_space_between(self):
        tree = self.get_tree()
        left = find_by_text(tree, "sb-left")
        right = find_by_text(tree, "sb-right")
        self.assertTrue(len(left) > 0 and len(right) > 0)
        self.assertLess(get_bounds(left[0])["x"], get_bounds(right[0])["x"])

    # --- AlignItems ---

    def test_align_start(self):
        tree = self.get_tree()
        nodes = find_by_text(tree, "align-start-item")
        self.assertTrue(len(nodes) > 0)

    def test_align_center(self):
        tree = self.get_tree()
        nodes = find_by_text(tree, "align-center-item")
        self.assertTrue(len(nodes) > 0)

    def test_align_end(self):
        tree = self.get_tree()
        nodes = find_by_text(tree, "align-end-item")
        self.assertTrue(len(nodes) > 0)

    # --- ExpansionBias ---

    def test_expansion_bias_proportional(self):
        tree = self.get_tree()
        e1 = find_by_text(tree, "expand-1x")
        e2 = find_by_text(tree, "expand-2x")
        e_none = find_by_text(tree, "expand-none")
        self.assertTrue(len(e1) > 0 and len(e2) > 0 and len(e_none) > 0)
        h1 = get_bounds(e1[0])["h"]
        h2 = get_bounds(e2[0])["h"]
        h_none = get_bounds(e_none[0])["h"]
        # 2x should be taller than 1x
        self.assertGreater(h2, h1 * 1.3, "expand-2x should be significantly taller than expand-1x")
        # both should be taller than no-expansion
        self.assertGreater(h1, h_none)

    # --- Grid colspan ---

    def test_grid_colspan(self):
        tree = self.get_tree()
        span = find_by_text(tree, "colspan-2-cell")
        normal = find_by_text(tree, "normal-cell")
        self.assertTrue(len(span) > 0 and len(normal) > 0)
        span_w = get_bounds(span[0])["w"]
        normal_w = get_bounds(normal[0])["w"]
        self.assertGreater(span_w, normal_w * 1.5, "colspan-2 cell should be wider")

    # --- Nested layout ---

    def test_nested_layout_exists(self):
        tree = self.get_tree()
        for name in ["nested-left-top", "nested-left-bottom", "nested-right-top", "nested-deep-a", "nested-deep-b"]:
            nodes = find_by_text(tree, name)
            self.assertTrue(len(nodes) > 0, f"{name} not found")

    def test_nested_deep_horizontal(self):
        tree = self.get_tree()
        a = find_by_text(tree, "nested-deep-a")[0]
        b = find_by_text(tree, "nested-deep-b")[0]
        self.assertLess(get_bounds(a)["x"], get_bounds(b)["x"])


if __name__ == "__main__":
    unittest.main()
