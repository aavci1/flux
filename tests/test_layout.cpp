#include <catch2/catch_test_macros.hpp>
#include <Flux/Layout/LayoutEngine.hpp>
#include <cmath>

using namespace flux;

static bool approx(float a, float b, float eps = 0.1f) {
    return std::abs(a - b) < eps;
}

TEST_CASE("VStack distributes children vertically", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 30} },
        { .intrinsicSize = {100, 30} },
        { .intrinsicSize = {100, 30} },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::start, AlignItems::stretch, {}, bounds
    );
    REQUIRE(rects.size() == 3);
    CHECK(approx(rects[0].y, 0));
    CHECK(approx(rects[1].y, 30));
    CHECK(approx(rects[2].y, 60));
    CHECK(approx(rects[0].width, 200));
}

TEST_CASE("HStack distributes children horizontally", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {50, 30} },
        { .intrinsicSize = {70, 30} },
    };
    Rect bounds = {0, 0, 200, 100};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Horizontal, children, 0, JustifyContent::start, AlignItems::stretch, {}, bounds
    );
    REQUIRE(rects.size() == 2);
    CHECK(approx(rects[0].x, 0));
    CHECK(approx(rects[1].x, 50));
    CHECK(approx(rects[0].height, 100));
}

TEST_CASE("Stack spacing is applied", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 20} },
        { .intrinsicSize = {100, 20} },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 10, JustifyContent::start, AlignItems::stretch, {}, bounds
    );
    CHECK(approx(rects[0].y, 0));
    CHECK(approx(rects[1].y, 30));
}

TEST_CASE("Stack padding is respected", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 20} },
    };
    EdgeInsets pad = {10, 20, 10, 20};
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::start, AlignItems::stretch, pad, bounds
    );
    CHECK(approx(rects[0].x, 20));
    CHECK(approx(rects[0].y, 10));
    CHECK(approx(rects[0].width, 160));
}

TEST_CASE("Expansion bias distributes extra space", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 20}, .expansionBias = 1.0f },
        { .intrinsicSize = {100, 20}, .expansionBias = 0.0f },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::start, AlignItems::stretch, {}, bounds
    );
    CHECK(approx(rects[0].height, 180));
    CHECK(approx(rects[1].height, 20));
}

TEST_CASE("JustifyContent::center centers children", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 50} },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::center, AlignItems::stretch, {}, bounds
    );
    CHECK(approx(rects[0].y, 75));
}

TEST_CASE("JustifyContent::end aligns to end", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 50} },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::end, AlignItems::stretch, {}, bounds
    );
    CHECK(approx(rects[0].y, 150));
}

TEST_CASE("AlignItems::center centers on cross axis", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {50, 30} },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::start, AlignItems::center, {}, bounds
    );
    CHECK(approx(rects[0].x, 75));
    CHECK(approx(rects[0].width, 50));
}

TEST_CASE("Invisible children are skipped", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 30}, .visible = true },
        { .intrinsicSize = {100, 30}, .visible = false },
        { .intrinsicSize = {100, 30}, .visible = true },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 10, JustifyContent::start, AlignItems::stretch, {}, bounds
    );
    CHECK(approx(rects[0].y, 0));
    CHECK(approx(rects[2].y, 40));
}

TEST_CASE("Grid places children in cells", "[layout]") {
    std::vector<GridChildInput> children = {
        { .colspan = 1, .rowspan = 1 },
        { .colspan = 1, .rowspan = 1 },
        { .colspan = 1, .rowspan = 1 },
        { .colspan = 1, .rowspan = 1 },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeGrid(children, 2, 2, 0, {}, bounds);
    REQUIRE(rects.size() == 4);
    CHECK(approx(rects[0].x, 0));   CHECK(approx(rects[0].y, 0));
    CHECK(approx(rects[1].x, 100)); CHECK(approx(rects[1].y, 0));
    CHECK(approx(rects[2].x, 0));   CHECK(approx(rects[2].y, 100));
    CHECK(approx(rects[3].x, 100)); CHECK(approx(rects[3].y, 100));
    CHECK(approx(rects[0].width, 100));
    CHECK(approx(rects[0].height, 100));
}

TEST_CASE("Grid handles colspan", "[layout]") {
    std::vector<GridChildInput> children = {
        { .colspan = 2, .rowspan = 1 },
        { .colspan = 1, .rowspan = 1 },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeGrid(children, 2, 2, 0, {}, bounds);
    CHECK(approx(rects[0].width, 200));
    CHECK(approx(rects[1].width, 100));
}

TEST_CASE("Min/max constraints are respected", "[layout]") {
    std::vector<StackChildInput> children = {
        { .intrinsicSize = {100, 10}, .expansionBias = 1.0f, .maxHeight = 50 },
        { .intrinsicSize = {100, 10}, .expansionBias = 1.0f },
    };
    Rect bounds = {0, 0, 200, 200};
    auto rects = LayoutEngine::computeStack(
        StackAxis::Vertical, children, 0, JustifyContent::start, AlignItems::stretch, {}, bounds
    );
    CHECK(rects[0].height <= 50.1f);
}
