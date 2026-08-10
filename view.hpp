#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <array>
#include <string>
#include <utility>

// #include "game.hpp"

using namespace ftxui;

namespace ms {

template<class Board>
struct View {
    static constexpr int ROWS = Board::nj;
    static constexpr int COLS = Board::ni;
    static constexpr int TILE_W = 4;
    static constexpr int TILE_H = 2;

    Board b;

    std::array<std::array<bool, COLS>, ROWS> clicked{};
    std::array<std::array<Box, COLS>, ROWS> tile_boxes{};

    int mouse_x = -1;
    int mouse_y = -1;
    int tile_x = -1;
    int tile_y = -1;

    Component renderer;
    Component comp;

    explicit View(Board board)
        : b(std::move(board)),
          renderer(Renderer([this] {
        Elements rows;

        for (int y = 0; y < ROWS; ++y) {
            Elements row;

            for (int x = 0; x < COLS; ++x) {
                Color bg = clicked[y][x]
                    ? Color::RGB(100, 100, 100)
                    : Color::RGB(200, 200, 200);

                row.push_back(
                    text("    ")
                    | bgcolor(bg)
                    | size(WIDTH, EQUAL, TILE_W)
                    | size(HEIGHT, EQUAL, TILE_H)
                    | reflect(tile_boxes[y][x])
                );
            }

            rows.push_back(hbox(std::move(row)));
        }

        auto board = vbox(std::move(rows));

        return vbox({
            board,

            separator(),

            text(
                "mouse = (" +
                std::to_string(mouse_x) + ", " +
                std::to_string(mouse_y) + ")"
            ),

            text(
                "tile = (" +
                std::to_string(tile_x) + ", " +
                std::to_string(tile_y) + ")"
            ),
        });
    })),
          comp(CatchEvent(renderer, [this](Event event) {
        if (!event.is_mouse())
            return false;

        const Mouse& mouse = event.mouse();

        mouse_x = mouse.x;
        mouse_y = mouse.y;

        tile_x = -1;
        tile_y = -1;

        for (int y = 0; y < ROWS; ++y) {
            for (int x = 0; x < COLS; ++x) {
                if (tile_boxes[y][x].Contain(mouse.x, mouse.y)) {
                    tile_x = x;
                    tile_y = y;
                    break;
                }
            }

            if (tile_x != -1) {
                break;
            }
        }

        if (tile_x != -1 &&
            mouse.button == Mouse::Left &&
            mouse.motion == Mouse::Pressed) {
            clicked[tile_y][tile_x] = true;
        }

        return true;
    })) {}
};

template<class Board>
auto make_view(Board board) -> View<Board> {
    return View<Board>(std::move(board));
}

}