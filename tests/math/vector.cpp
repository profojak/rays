module;

#include <doctest/doctest.h>

module rays;

import :type;
import :vector;

TEST_CASE("`Vector` default constructor") {
    rays::Vector3f v;
    CHECK(v[0] == doctest::Approx(0.0f));
    CHECK(v[1] == doctest::Approx(0.0f));
    CHECK(v[2] == doctest::Approx(0.0f));
}

TEST_CASE("`Vector` element access") {
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    CHECK(v[0] == doctest::Approx(1.0f));
    CHECK(v[1] == doctest::Approx(2.0f));
    CHECK(v[2] == doctest::Approx(3.0f));
}

TEST_CASE("`Vector` addition") {
    rays::Vector3f a{1.0f, 2.0f, 3.0f};
    rays::Vector3f b{4.0f, 5.0f, 6.0f};
    auto c = a + b;

    CHECK(c[0] == doctest::Approx(5.0f));
    CHECK(c[1] == doctest::Approx(7.0f));
    CHECK(c[2] == doctest::Approx(9.0f));
}

TEST_CASE("`Vector` subtraction") {
    rays::Vector3f a{4.0f, 5.0f, 6.0f};
    rays::Vector3f b{1.0f, 2.0f, 3.0f};
    auto c = a - b;

    CHECK(c[0] == doctest::Approx(3.0f));
    CHECK(c[1] == doctest::Approx(3.0f));
    CHECK(c[2] == doctest::Approx(3.0f));
}

TEST_CASE("`Vector` scalar multiplication") {
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    auto scaled = v * 2.0f;

    CHECK(scaled[0] == doctest::Approx(2.0f));
    CHECK(scaled[1] == doctest::Approx(4.0f));
    CHECK(scaled[2] == doctest::Approx(6.0f));
}

TEST_CASE("`Vector` scalar division") {
    rays::Vector3f v{2.0f, 4.0f, 6.0f};
    auto divided = v / 2.0f;

    CHECK(divided[0] == doctest::Approx(1.0f));
    CHECK(divided[1] == doctest::Approx(2.0f));
    CHECK(divided[2] == doctest::Approx(3.0f));
}

TEST_CASE("`Vector` unary minus") {
    rays::Vector3f v{1.0f, -2.0f, 3.0f};
    auto negated = -v;

    CHECK(negated[0] == doctest::Approx(-1.0f));
    CHECK(negated[1] == doctest::Approx(2.0f));
    CHECK(negated[2] == doctest::Approx(-3.0f));
}

TEST_CASE("`Vector` addition assignment") {
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    auto &after = v += rays::Vector3f{1.0f, 1.0f, 1.0f};

    CHECK(after[0] == doctest::Approx(2.0f));
    CHECK(after[1] == doctest::Approx(3.0f));
    CHECK(after[2] == doctest::Approx(4.0f));
    CHECK(&after == &v);
}

TEST_CASE("`Vector` multiplication assignment") {
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    auto &after = v *= 2.0f;

    CHECK(after[0] == doctest::Approx(2.0f));
    CHECK(after[1] == doctest::Approx(4.0f));
    CHECK(after[2] == doctest::Approx(6.0f));
    CHECK(&after == &v);
}

TEST_CASE("`Vector` division assignment") {
    rays::Vector3f v{2.0f, 4.0f, 6.0f};
    auto &after = v /= 2.0f;

    CHECK(after[0] == doctest::Approx(1.0f));
    CHECK(after[1] == doctest::Approx(2.0f));
    CHECK(after[2] == doctest::Approx(3.0f));
    CHECK(&after == &v);
}

TEST_CASE("`Vector` scalar addition") {
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    auto result = v + 1.0f;

    CHECK(result[0] == doctest::Approx(2.0f));
    CHECK(result[1] == doctest::Approx(3.0f));
    CHECK(result[2] == doctest::Approx(4.0f));
}

TEST_CASE("`Vector` scalar subtraction") {
    rays::Vector3f v{2.0f, 3.0f, 4.0f};
    auto result = v - 1.0f;

    CHECK(result[0] == doctest::Approx(1.0f));
    CHECK(result[1] == doctest::Approx(2.0f));
    CHECK(result[2] == doctest::Approx(3.0f));
}

TEST_CASE("`Vector` scalar addition assignment") {
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    auto &after = v += 1.0f;

    CHECK(after[0] == doctest::Approx(2.0f));
    CHECK(after[1] == doctest::Approx(3.0f));
    CHECK(after[2] == doctest::Approx(4.0f));
    CHECK(&after == &v);
}

TEST_CASE("`Vector` scalar subtraction assignment") {
    rays::Vector3f v{2.0f, 3.0f, 4.0f};
    auto &after = v -= 1.0f;

    CHECK(after[0] == doctest::Approx(1.0f));
    CHECK(after[1] == doctest::Approx(2.0f));
    CHECK(after[2] == doctest::Approx(3.0f));
    CHECK(&after == &v);
}
