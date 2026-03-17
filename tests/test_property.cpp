#include <catch2/catch_test_macros.hpp>
#include <Flux/Core/Property.hpp>
#include <string>
#include <vector>

using namespace flux;

// Stub out the global redraw request so Property<T> works in tests
namespace flux {
void requestApplicationRedraw() {}
}

// --- Basic Property<int> ---

TEST_CASE("Property<int> default construction", "[property]") {
    Property<int> p;
    REQUIRE(p.get() == 0);
}

TEST_CASE("Property<int> value construction", "[property]") {
    Property<int> p = 42;
    REQUIRE(p.get() == 42);
}

TEST_CASE("Property<int> assignment", "[property]") {
    Property<int> p = 0;
    p = 99;
    REQUIRE(p.get() == 99);
}

TEST_CASE("Property<int> implicit conversion", "[property]") {
    Property<int> p = 7;
    int val = p;
    REQUIRE(val == 7);
}

TEST_CASE("Property<int> increment/decrement", "[property]") {
    Property<int> p = 10;
    ++p;
    REQUIRE(p.get() == 11);
    p++;
    REQUIRE(p.get() == 12);
    --p;
    REQUIRE(p.get() == 11);
    p--;
    REQUIRE(p.get() == 10);
}

TEST_CASE("Property<int> compound assignment", "[property]") {
    Property<int> p = 10;
    p += 5;
    REQUIRE(p.get() == 15);
    p -= 3;
    REQUIRE(p.get() == 12);
}

TEST_CASE("Property<int> comparison", "[property]") {
    Property<int> p = 10;
    REQUIRE(p == 10);
    REQUIRE(p != 9);
    REQUIRE(p > 5);
    REQUIRE(p < 15);
    REQUIRE(p >= 10);
    REQUIRE(p <= 10);
}

TEST_CASE("Property<int> arithmetic", "[property]") {
    Property<int> p = 10;
    REQUIRE(p + 5 == 15);
    REQUIRE(p - 3 == 7);
    REQUIRE(p * 2 == 20);
    REQUIRE(p / 2 == 5);
}

// --- Property<float> ---

TEST_CASE("Property<float> construction", "[property]") {
    Property<float> p = 3.14f;
    REQUIRE(p.get() == 3.14f);
}

// --- Property<std::string> ---

TEST_CASE("Property<string> construction and assignment", "[property]") {
    Property<std::string> p = std::string("hello");
    REQUIRE(p.get() == "hello");
    p = std::string("world");
    REQUIRE(p.get() == "world");
}

TEST_CASE("Property<string> from char literal via deduction guide", "[property]") {
    Property p("hello");
    REQUIRE(p.get() == "hello");
}

// --- Property with lambda ---

TEST_CASE("Property<int> from lambda", "[property]") {
    int counter = 42;
    Property<int> p = [&counter]() { return counter; };
    REQUIRE(p.get() == 42);
    counter = 100;
    REQUIRE(p.get() == 100);
}

// --- Property<bool> ---

TEST_CASE("Property<bool> toggle behavior", "[property]") {
    Property<bool> p = false;
    REQUIRE_FALSE(static_cast<bool>(p));
    p = true;
    REQUIRE(static_cast<bool>(p));
}

// --- Property copy sharing ---

TEST_CASE("Property copies share storage", "[property]") {
    Property<int> a = 10;
    Property<int> b = a;
    a = 20;
    REQUIRE(b.get() == 20);
}

// --- Property<vector> ---

TEST_CASE("Property<vector<int>> construction", "[property]") {
    Property<std::vector<int>> p = std::vector<int>{1, 2, 3};
    auto v = p.get();
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
}

TEST_CASE("Property<vector<int>> assignment", "[property]") {
    Property<std::vector<int>> p;
    p = std::vector<int>{4, 5, 6};
    auto v = p.get();
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 4);
}
