module;

#include <doctest/doctest.h>

module rays;

import :matrix;
import :type;
import :vector;

TEST_CASE("`Matrix` default constructor") {
    rays::Matrix3f m;
    CHECK(m[0, 0] == doctest::Approx(0.0f));
    CHECK(m[1, 2] == doctest::Approx(0.0f));
    CHECK(m[2, 2] == doctest::Approx(0.0f));
}

TEST_CASE("`Matrix` constructor from scalar") {
    rays::Matrix2f m{5.0f};
    CHECK(m[0, 0] == doctest::Approx(5.0f));
    CHECK(m[0, 1] == doctest::Approx(5.0f));
    CHECK(m[1, 0] == doctest::Approx(5.0f));
    CHECK(m[1, 1] == doctest::Approx(5.0f));
}

TEST_CASE("`Matrix` constructor from scalars") {
    rays::Matrix2f m{1.0f, 2.0f, 3.0f, 4.0f};
    CHECK(m[0, 0] == doctest::Approx(1.0f));
    CHECK(m[0, 1] == doctest::Approx(2.0f));
    CHECK(m[1, 0] == doctest::Approx(3.0f));
    CHECK(m[1, 1] == doctest::Approx(4.0f));
}

TEST_CASE("`Matrix` element access by row and column") {
    rays::Matrix3f m{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    CHECK(m[0, 2] == doctest::Approx(3.0f));
    CHECK(m[1, 1] == doctest::Approx(5.0f));
    CHECK(m[2, 0] == doctest::Approx(7.0f));
}

TEST_CASE("`Matrix` element access by index") {
    rays::Matrix3f m{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    CHECK(m[0] == doctest::Approx(1.0f));
    CHECK(m[4] == doctest::Approx(5.0f));
    CHECK(m[8] == doctest::Approx(9.0f));
}

TEST_CASE("`Matrix` addition") {
    rays::Matrix2f a{1.0f, 2.0f, 3.0f, 4.0f};
    rays::Matrix2f b{4.0f, 5.0f, 6.0f, 7.0f};
    auto c = a + b;

    CHECK(c[0, 0] == doctest::Approx(5.0f));
    CHECK(c[0, 1] == doctest::Approx(7.0f));
    CHECK(c[1, 0] == doctest::Approx(9.0f));
    CHECK(c[1, 1] == doctest::Approx(11.0f));
}

TEST_CASE("`Matrix` subtraction") {
    rays::Matrix2f a{4.0f, 5.0f, 6.0f, 7.0f};
    rays::Matrix2f b{1.0f, 2.0f, 3.0f, 4.0f};
    auto c = a - b;

    CHECK(c[0, 0] == doctest::Approx(3.0f));
    CHECK(c[0, 1] == doctest::Approx(3.0f));
    CHECK(c[1, 0] == doctest::Approx(3.0f));
    CHECK(c[1, 1] == doctest::Approx(3.0f));
}

TEST_CASE("`Matrix` scalar multiplication") {
    rays::Matrix2f m{1.0f, 2.0f, 3.0f, 4.0f};
    auto scaled = m * 2.0f;

    CHECK(scaled[0, 0] == doctest::Approx(2.0f));
    CHECK(scaled[0, 1] == doctest::Approx(4.0f));
    CHECK(scaled[1, 0] == doctest::Approx(6.0f));
    CHECK(scaled[1, 1] == doctest::Approx(8.0f));
}

TEST_CASE("`Matrix` scalar division") {
    rays::Matrix2f m{2.0f, 4.0f, 6.0f, 8.0f};
    auto divided = m / 2.0f;

    CHECK(divided[0, 0] == doctest::Approx(1.0f));
    CHECK(divided[0, 1] == doctest::Approx(2.0f));
    CHECK(divided[1, 0] == doctest::Approx(3.0f));
    CHECK(divided[1, 1] == doctest::Approx(4.0f));
}

TEST_CASE("`Matrix` unary minus") {
    rays::Matrix2f m{1.0f, -2.0f, 3.0f, -4.0f};
    auto negated = -m;

    CHECK(negated[0, 0] == doctest::Approx(-1.0f));
    CHECK(negated[0, 1] == doctest::Approx(2.0f));
    CHECK(negated[1, 0] == doctest::Approx(-3.0f));
    CHECK(negated[1, 1] == doctest::Approx(4.0f));
}

TEST_CASE("`Matrix` matrix-vector product") {
    rays::Matrix<rays::Float, 2, 3> m{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    rays::Vector3f v{1.0f, 2.0f, 3.0f};
    auto result = m * v;

    CHECK(result[0] == doctest::Approx(14.0f));
    CHECK(result[1] == doctest::Approx(32.0f));
}

TEST_CASE("`Matrix` matrix-matrix product") {
    rays::Matrix<rays::Float, 2, 3> a{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    rays::Matrix<rays::Float, 3, 2> b{7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    auto c = a * b;

    CHECK(c[0, 0] == doctest::Approx(58.0f));
    CHECK(c[0, 1] == doctest::Approx(64.0f));
    CHECK(c[1, 0] == doctest::Approx(139.0f));
    CHECK(c[1, 1] == doctest::Approx(154.0f));
}

TEST_CASE("`Matrix` transpose") {
    rays::Matrix<rays::Float, 2, 3> m{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    auto transposed = m.Transposed();

    CHECK(transposed[0, 0] == doctest::Approx(1.0f));
    CHECK(transposed[0, 1] == doctest::Approx(4.0f));
    CHECK(transposed[1, 0] == doctest::Approx(2.0f));
    CHECK(transposed[1, 1] == doctest::Approx(5.0f));
    CHECK(transposed[2, 0] == doctest::Approx(3.0f));
    CHECK(transposed[2, 1] == doctest::Approx(6.0f));
}

TEST_CASE("`Matrix` identity") {
    auto m = rays::Matrix3f::Identity();

    CHECK(m[0, 0] == doctest::Approx(1.0f));
    CHECK(m[0, 1] == doctest::Approx(0.0f));
    CHECK(m[1, 1] == doctest::Approx(1.0f));
    CHECK(m[1, 2] == doctest::Approx(0.0f));
    CHECK(m[2, 2] == doctest::Approx(1.0f));
    CHECK(m[2, 0] == doctest::Approx(0.0f));
}

TEST_CASE("`Matrix` conversion from arithmetic type") {
    rays::Matrix2f f{1.0f, 2.0f, 3.0f, 4.0f};
    rays::Matrix<rays::Double, 2, 2> d{f};

    CHECK(d[0, 0] == doctest::Approx(1.0));
    CHECK(d[1, 1] == doctest::Approx(4.0));
}

TEST_CASE("`Matrix` addition assignment") {
    rays::Matrix2f m{1.0f, 2.0f, 3.0f, 4.0f};
    m += rays::Matrix2f{1.0f, 1.0f, 1.0f, 1.0f};

    CHECK(m[0, 0] == doctest::Approx(2.0f));
    CHECK(m[0, 1] == doctest::Approx(3.0f));
    CHECK(m[1, 0] == doctest::Approx(4.0f));
    CHECK(m[1, 1] == doctest::Approx(5.0f));
}

TEST_CASE("`Matrix` multiplication assignment") {
    rays::Matrix2f m{1.0f, 2.0f, 3.0f, 4.0f};
    m *= 2.0f;

    CHECK(m[0, 0] == doctest::Approx(2.0f));
    CHECK(m[0, 1] == doctest::Approx(4.0f));
    CHECK(m[1, 0] == doctest::Approx(6.0f));
    CHECK(m[1, 1] == doctest::Approx(8.0f));
}

TEST_CASE("`Matrix` division assignment") {
    rays::Matrix2f m{2.0f, 4.0f, 6.0f, 8.0f};
    m /= 2.0f;

    CHECK(m[0, 0] == doctest::Approx(1.0f));
    CHECK(m[0, 1] == doctest::Approx(2.0f));
    CHECK(m[1, 0] == doctest::Approx(3.0f));
    CHECK(m[1, 1] == doctest::Approx(4.0f));
}

TEST_CASE("`Matrix` equality") {
    rays::Matrix2f a{1.0f, 2.0f, 3.0f, 4.0f};
    rays::Matrix2f b{1.0f, 2.0f, 3.0f, 4.0f};
    rays::Matrix2f c{4.0f, 3.0f, 2.0f, 1.0f};

    CHECK(a == b);
    CHECK_FALSE(a == c);
}
