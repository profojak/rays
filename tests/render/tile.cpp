module;

#include <doctest/doctest.h>

module rays;

import :film;
import :pixel;
import :scheduler;
import :tile;
import :type;

TEST_CASE("`Tile::PixelAt` accepts absolute film coordinates") {
    const rays::Bounds2u bounds{rays::Point2u{4, 2}, rays::UInt{2}};
    rays::Tile<float> tile{bounds};

    // Cover every pixel in tile using absolute coordinates.
    for (rays::UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
        for (rays::UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
            tile.PixelAt(x, y) = rays::Pixel<float>{1.0f, 0.5f, 0.25f};
        }
    }

    CHECK(tile.PixelAt(4, 2).R() == 1.0f);
    CHECK(tile.PixelAt(5, 3).G() == 0.5f);
}

TEST_CASE("`Film::PutTile` writes to correct region") {
    const rays::Vector2u resolution{6, 6};
    rays::Film<float> film{resolution};

    const rays::Bounds2u bounds{rays::Point2u{2, 3}, rays::UInt{2}};
    rays::Tile<float> tile{bounds};
    for (rays::UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
        for (rays::UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
            tile.PixelAt(x, y) = rays::Pixel<float>{1.0f, 1.0f, 1.0f};
        }
    }

    film.PutTile(tile);

    for (rays::UInt y = 0; y < resolution[1]; ++y) {
        for (rays::UInt x = 0; x < resolution[0]; ++x) {
            const auto &pixel = film.PixelAt(x, y);
            const bool in_tile = x >= bounds.min[0] && x < bounds.max[0] &&
                                 y >= bounds.min[1] && y < bounds.max[1];
            if (in_tile) {
                CHECK(pixel.R() == 1.0f);
            } else {
                CHECK(pixel.R() == 0.0f);
            }
        }
    }
}

TEST_CASE("`SpiralScheduler` tiles fully cover non-square film") {
    const rays::Vector2u resolution{20, 12};
    const rays::UInt block_size = 2;
    rays::SpiralScheduler<float> scheduler{resolution, block_size};
    rays::Film<float> film{resolution};

    std::size_t tiles = 0;
    std::optional<rays::Tile<float>> tile;
    while ((tile = scheduler.NextTile()).has_value()) {
        const auto &bounds = tile->Bounds();
        for (rays::UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
            for (rays::UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
                tile->PixelAt(x, y) = rays::Pixel<float>{1.0f, 0.0f, 0.0f};
            }
        }
        film.PutTile(*tile);
        ++tiles;
    }

    CHECK(tiles == (20u / 2u) * (12u / 2u));

    // Every pixel of film must have been written exactly once.
    for (rays::UInt y = 0; y < resolution[1]; ++y) {
        for (rays::UInt x = 0; x < resolution[0]; ++x) {
            CHECK(film.PixelAt(x, y).R() == 1.0f);
        }
    }
}
