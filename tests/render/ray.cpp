module;

#include <doctest/doctest.h>

module rays;

import :ray;
import :type;
import :vector;

TEST_CASE("`Ray` default constructor") {
    rays::Ray3f ray;

    CHECK(ray.origin[0] == doctest::Approx(0.0f));
    CHECK(ray.origin[1] == doctest::Approx(0.0f));
    CHECK(ray.origin[2] == doctest::Approx(0.0f));
    CHECK(ray.direction[0] == doctest::Approx(0.0f));
    CHECK(ray.direction[1] == doctest::Approx(0.0f));
    CHECK(ray.direction[2] == doctest::Approx(0.0f));
}

TEST_CASE("`Ray` aggregate initialization") {
    const rays::Ray3f ray{{1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, -1.0f}};

    CHECK(ray.origin[0] == doctest::Approx(1.0f));
    CHECK(ray.origin[1] == doctest::Approx(2.0f));
    CHECK(ray.origin[2] == doctest::Approx(3.0f));
    CHECK(ray.direction[0] == doctest::Approx(0.0f));
    CHECK(ray.direction[1] == doctest::Approx(0.0f));
    CHECK(ray.direction[2] == doctest::Approx(-1.0f));
}
