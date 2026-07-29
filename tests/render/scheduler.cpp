module;

#include <doctest/doctest.h>

module rays;

import :scheduler;
import :type;

TEST_CASE("`SpiralScheduler` returns expected number of tiles") {
    rays::SpiralScheduler scheduler{rays::Vector2u{4, 4}, 2};

    std::size_t count = 0;
    while (scheduler.NextTile().has_value()) {
        ++count;
    }

    CHECK(count == 4);
}

TEST_CASE("`SpiralScheduler` returns `nullopt` after exhaustion") {
    rays::SpiralScheduler scheduler{rays::Vector2u{4, 4}, 2};

    for (std::size_t i = 0; i < 4; ++i) {
        CHECK(scheduler.NextTile().has_value());
    }
    CHECK_FALSE(scheduler.NextTile().has_value());
    CHECK_FALSE(scheduler.NextTile().has_value());
}

TEST_CASE("`SpiralScheduler` tiles cover whole film") {
    const rays::Vector2u film_resolution{6, 6};
    const rays::UInt block_size = 2;
    rays::SpiralScheduler scheduler{film_resolution, block_size};

    std::size_t total_area = 0;
    std::optional<rays::Tile> tile;
    while ((tile = scheduler.NextTile()).has_value()) {
        const auto &bounds = tile->Bounds();
        total_area += static_cast<std::size_t>(bounds.Size(0)) *
                      static_cast<std::size_t>(bounds.Size(1));
    }

    const std::size_t film_area = static_cast<std::size_t>(film_resolution[0]) *
                                  static_cast<std::size_t>(film_resolution[1]);
    CHECK(total_area == film_area);
}

TEST_CASE("`SpiralScheduler` starts near film center") {
    rays::SpiralScheduler scheduler{rays::Vector2u{8, 8}, 2};

    const auto first_tile = scheduler.NextTile();
    REQUIRE(first_tile.has_value());

    const auto &bounds = first_tile->Bounds();
    CHECK(bounds.Size(0) == 2);
    CHECK(bounds.Size(1) == 2);

    CHECK(bounds.min[0] >= 2);
    CHECK(bounds.min[1] >= 2);
    CHECK(bounds.max[0] <= 6);
    CHECK(bounds.max[1] <= 6);
}

TEST_CASE("`SpiralScheduler` clamps edges to film resolution") {
    const rays::Vector2u film_resolution{15, 13};
    const rays::UInt block_size = 4;
    rays::SpiralScheduler scheduler{film_resolution, block_size};

    std::size_t total_area = 0;
    bool found_right_edge = false;
    bool found_bottom_edge = false;
    bool found_corner_tile = false;

    std::optional<rays::Tile> tile;
    while ((tile = scheduler.NextTile()).has_value()) {
        const auto &bounds = tile->Bounds();

        CHECK(bounds.max[0] <= film_resolution[0]);
        CHECK(bounds.max[1] <= film_resolution[1]);
        CHECK(bounds.Size(0) > 0);
        CHECK(bounds.Size(1) > 0);

        total_area += static_cast<std::size_t>(bounds.Size(0)) *
                      static_cast<std::size_t>(bounds.Size(1));

        const bool clamps_right =
            bounds.max[0] == film_resolution[0] && bounds.Size(0) < block_size;
        const bool clamps_bottom =
            bounds.max[1] == film_resolution[1] && bounds.Size(1) < block_size;

        if (clamps_right) {
            found_right_edge = true;
        }
        if (clamps_bottom) {
            found_bottom_edge = true;
        }
        if (clamps_right && clamps_bottom) {
            found_corner_tile = true;
        }
    }

    const std::size_t film_area = static_cast<std::size_t>(film_resolution[0]) *
                                  static_cast<std::size_t>(film_resolution[1]);
    CHECK(total_area == film_area);
    CHECK(found_right_edge);
    CHECK(found_bottom_edge);
    CHECK(found_corner_tile);
}

TEST_CASE("`SpiralScheduler` can be reset and run again") {
    rays::SpiralScheduler scheduler{rays::Vector2u{4, 4}, 2};

    for (std::size_t i = 0; i < 4; ++i) {
        CHECK(scheduler.NextTile().has_value());
    }
    CHECK_FALSE(scheduler.NextTile().has_value());

    scheduler.Reset();

    for (std::size_t i = 0; i < 4; ++i) {
        CHECK(scheduler.NextTile().has_value());
    }
    CHECK_FALSE(scheduler.NextTile().has_value());
}

TEST_CASE("`SpiralScheduler` handles single-tile film") {
    rays::SpiralScheduler scheduler{rays::Vector2u{2, 2}, 2};

    const auto tile = scheduler.NextTile();
    REQUIRE(tile.has_value());
    CHECK(tile->Bounds().Size(0) == 2);
    CHECK(tile->Bounds().Size(1) == 2);
    CHECK_FALSE(scheduler.NextTile().has_value());
}

TEST_CASE("`SpiralScheduler` handles film smaller than block size") {
    rays::SpiralScheduler scheduler{rays::Vector2u{1, 1}, 4};

    const auto tile = scheduler.NextTile();
    REQUIRE(tile.has_value());
    CHECK(tile->Bounds().Size(0) == 1);
    CHECK(tile->Bounds().Size(1) == 1);
    CHECK_FALSE(scheduler.NextTile().has_value());
}
