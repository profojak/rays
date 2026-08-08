module;

#include <doctest/doctest.h>

#include <optional>

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

    const auto hit = triangle.Intersect(ray, v0, v1, v2);
    REQUIRE(hit.has_value());
    CHECK(hit->t == doctest::Approx(1.0f));
    CHECK(hit->uv[0] == doctest::Approx(0.25f));
    CHECK(hit->uv[1] == doctest::Approx(0.5f));
}

TEST_CASE("`Triangle::Intersect` misses triangle behind origin") {
    const rays::Ray3f ray{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2));
}

TEST_CASE("`Triangle::Intersect` misses parallel ray") {
    const rays::Ray3f ray{{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2));
}

TEST_CASE("`Triangle::Intersect` misses ray outside triangle") {
    const rays::Ray3f ray{{3.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2));
}

TEST_CASE("`Triangle::Intersect` misses ray past triangle") {
    const rays::Ray3f ray{{0.0f, 0.0f, -2.0f}, {0.0f, 0.0f, -1.0f}};

    CHECK_FALSE(triangle.Intersect(ray, v0, v1, v2));
}

TEST_CASE("`Triangle::Intersect` hits edge of triangle") {
    const rays::Ray3f ray{{0.0f, -1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}};

    const auto hit = triangle.Intersect(ray, v0, v1, v2);
    REQUIRE(hit.has_value());
    CHECK(hit->t == doctest::Approx(1.0f));
    CHECK(hit->uv[0] == doctest::Approx(0.5f));
    CHECK(hit->uv[1] == doctest::Approx(0.0f));
}
