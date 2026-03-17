#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <Flux/Core/Types.hpp>

using namespace flux;
using Catch::Matchers::WithinAbs;

// --- Point ---

TEST_CASE("Point default construction", "[types][point]") {
    Point p;
    REQUIRE(p.x == 0.0f);
    REQUIRE(p.y == 0.0f);
}

TEST_CASE("Point value construction", "[types][point]") {
    Point p{3.0f, 4.0f};
    REQUIRE(p.x == 3.0f);
    REQUIRE(p.y == 4.0f);
}

// --- Size ---

TEST_CASE("Size default construction", "[types][size]") {
    Size s;
    REQUIRE(s.width == 0.0f);
    REQUIRE(s.height == 0.0f);
}

TEST_CASE("Size value construction", "[types][size]") {
    Size s{100.0f, 200.0f};
    REQUIRE(s.width == 100.0f);
    REQUIRE(s.height == 200.0f);
}

// --- Rect ---

TEST_CASE("Rect default construction", "[types][rect]") {
    Rect r;
    REQUIRE(r.x == 0.0f);
    REQUIRE(r.y == 0.0f);
    REQUIRE(r.width == 0.0f);
    REQUIRE(r.height == 0.0f);
}

TEST_CASE("Rect contains point", "[types][rect]") {
    Rect r{10, 10, 100, 100};
    REQUIRE(r.contains({50, 50}));
    REQUIRE(r.contains({10, 10}));
    REQUIRE_FALSE(r.contains({5, 50}));
    REQUIRE_FALSE(r.contains({50, 5}));
    REQUIRE_FALSE(r.contains({111, 50}));
    REQUIRE_FALSE(r.contains({50, 111}));
}

// --- Color ---

TEST_CASE("Color from RGBA", "[types][color]") {
    Color c{1.0f, 0.5f, 0.25f, 0.75f};
    REQUIRE_THAT(c.r, WithinAbs(1.0, 0.001));
    REQUIRE_THAT(c.g, WithinAbs(0.5, 0.001));
    REQUIRE_THAT(c.b, WithinAbs(0.25, 0.001));
    REQUIRE_THAT(c.a, WithinAbs(0.75, 0.001));
}

TEST_CASE("Color from hex", "[types][color]") {
    Color c = Color::hex(0xFF0000);
    REQUIRE_THAT(c.r, WithinAbs(1.0, 0.01));
    REQUIRE_THAT(c.g, WithinAbs(0.0, 0.01));
    REQUIRE_THAT(c.b, WithinAbs(0.0, 0.01));
    REQUIRE_THAT(c.a, WithinAbs(1.0, 0.01));
}

// --- EdgeInsets ---

TEST_CASE("EdgeInsets uniform construction", "[types][edge_insets]") {
    EdgeInsets e(10);
    REQUIRE(e.top == 10.0f);
    REQUIRE(e.right == 10.0f);
    REQUIRE(e.bottom == 10.0f);
    REQUIRE(e.left == 10.0f);
}

TEST_CASE("EdgeInsets horizontal/vertical", "[types][edge_insets]") {
    EdgeInsets e{10, 20, 30, 40};
    REQUIRE_THAT(e.horizontal(), WithinAbs(60.0, 0.001));
    REQUIRE_THAT(e.vertical(), WithinAbs(40.0, 0.001));
}

// --- CornerRadius ---

TEST_CASE("CornerRadius uniform construction", "[types][corner_radius]") {
    CornerRadius cr{5, 5, 5, 5};
    REQUIRE(cr.topLeft == 5.0f);
    REQUIRE(cr.topRight == 5.0f);
    REQUIRE(cr.bottomRight == 5.0f);
    REQUIRE(cr.bottomLeft == 5.0f);
}
