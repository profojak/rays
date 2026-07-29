module;

#include <doctest/doctest.h>

module rays;

import :point;
import :type;

TEST_CASE("`Point` default constructor") {
    rays::Point2i p;
    CHECK(p[0] == 0);
    CHECK(p[1] == 0);
}

TEST_CASE("`Point` constructor from scalar") {
    rays::Point2i p{5};
    CHECK(p[0] == 5);
    CHECK(p[1] == 5);
}

TEST_CASE("`Point` constructor from multiple scalars") {
    rays::Point2i p{2, 3};
    CHECK(p[0] == 2);
    CHECK(p[1] == 3);
}

TEST_CASE("`Point` element access") {
    rays::Point2u p{7, 8};
    CHECK(p[0] == 7);
    CHECK(p[1] == 8);
}

TEST_CASE("`Point` addition") {
    rays::Point2i a{1, 2};
    rays::Point2i b{3, 4};
    auto c = a + b;

    CHECK(c[0] == 4);
    CHECK(c[1] == 6);
}

TEST_CASE("`Point` conversion from arithmetic type") {
    rays::Point2i int_point{1, 2};
    rays::Point<rays::Float, 2> float_point{int_point};

    CHECK(float_point[0] == doctest::Approx(1.0f));
    CHECK(float_point[1] == doctest::Approx(2.0f));
}
