module;

#include <doctest/doctest.h>

#include <limits>

module rays;

import :ray;
import :triangle;
import :type;
import :vector;

namespace {

/// Triangle with vertices in `z == 0` plane.
constexpr rays::Triangle triangle{0, 1, 2};

/// Vertex positions of triangle in `z == 0` plane.
const rays::Vector3f v0{-1.0f, -1.0f, 0.0f};
const rays::Vector3f v1{1.0f, -1.0f, 0.0f};
const rays::Vector3f v2{0.0f, 1.0f, 0.0f};

} // namespace

TEST_CASE("`Triangle::Intersect` hits triangle") {
    const rays::Ray3f ray{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};
    auto t = std::numeric_limits<rays::Float>::infinity();

    CHECK(triangle.Intersect(ray, v0, v1, v2, t));
    CHECK(t == doctest::Approx(1.0f));
}

TEST_CASE("`Triangle::Intersect` misses triangle behind origin") {
    const rays::Ray3f ray{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};
    auto t = std::numeric_limits<rays::Float>::infinity();

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2, t));
}

TEST_CASE("`Triangle::Intersect` misses parallel ray") {
    const rays::Ray3f ray{{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
    auto t = std::numeric_limits<rays::Float>::infinity();

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2, t));
}

TEST_CASE("`Triangle::Intersect` misses ray outside triangle") {
    const rays::Ray3f ray{{3.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};
    auto t = std::numeric_limits<rays::Float>::infinity();

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2, t));
}

TEST_CASE("`Triangle::Intersect` misses ray past triangle") {
    const rays::Ray3f ray{{0.0f, 0.0f, -2.0f}, {0.0f, 0.0f, -1.0f}};
    auto t = std::numeric_limits<rays::Float>::infinity();

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2, t));
}

TEST_CASE("`Triangle::Intersect` hits edge of triangle") {
    const rays::Ray3f ray{{0.0f, -1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};
    auto t = std::numeric_limits<rays::Float>::infinity();

    CHECK(triangle.Intersect(ray, v0, v1, v2, t));
    CHECK(t == doctest::Approx(1.0f));
}
