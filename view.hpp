#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <array>
#include <chrono>
#include <functional>
#include <string>
#include <utility>

// #include "game.hpp"

using namespace ftxui;

namespace ms {

template<class Board>
struct View {
    static constexpr int ROWS = Board::nj;
    static constexpr int COLS = Board::ni;
    static constexpr int TILE_W = 2;
    static constexpr int TILE_H = 1;

    enum class State { Playing, Won, Lost };

    Board b;
    State state = State::Playing;
    bool finished = false;
    bool timer_started = false;
    std::chrono::steady_clock::time_point timer_start{};
    std::chrono::seconds frozen_elapsed{};

    std::array<std::array<bool, COLS>, ROWS> clicked{};
    std::array<std::array<bool, COLS>, ROWS> flagged{};
    std::array<std::array<std::string, COLS>, ROWS> labels{};
    std::array<std::array<Box, COLS>, ROWS> tile_boxes{};

    int tile_x = -1;
    int tile_y = -1;

    Component renderer;
    Component menu_button;
    Component restart_button;
    Component comp;

    void restart() {
        state = State::Playing;
        finished = false;
        timer_started = false;
        frozen_elapsed = {};
        clicked = {};
        flagged = {};
        labels = {};
        tile_x = -1;
        tile_y = -1;
    }

    std::chrono::seconds elapsed() const {
        if (finished || !timer_started)
            return frozen_elapsed;

        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - timer_start);
    }

    std::string timer_text() const {
        const auto total_seconds = elapsed().count();
        const auto minutes = total_seconds / 60;
        const auto seconds = total_seconds % 60;
        return "Time: " + std::to_string(minutes) + ":" +
            (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }

    void finish(State result) {
        frozen_elapsed = elapsed();
        state = result;
        finished = true;
    }

    static Color label_color(const std::string& label) {
        if (label.empty())
            return Color::Default;

        switch (label.front()) {
            case '1': return Color::RGB(154, 201, 255);
            case '2': return Color::RGB(168, 230, 207);
            case '3': return Color::RGB(255, 173, 173);
            case '4': return Color::RGB(205, 180, 219);
            case '5': return Color::RGB(255, 199, 153);
            case '6': return Color::RGB(144, 224, 239);
            case '7': return Color::RGB(253, 230, 138);
            case '8': return Color::RGB(255, 198, 211);
            default: return Color::RGB(255, 128, 128);
        }
    }

        explicit View(Board board, std::function<void()> on_menu)
        : b(std::move(board)),
          renderer(Renderer([this] {
        Elements rows;

        for (int y = 0; y < ROWS; ++y) {
            Elements row;

            for (int x = 0; x < COLS; ++x) {
                Color bg = flagged[y][x]
                    ? Color::RGB(255, 173, 173)
                    : clicked[y][x]
                    ? Color::RGB(100, 100, 100)
                    : (x+y) % 2 == 0 ? Color::RGB(200, 200, 200) : Color::RGB(210, 210, 210);

                row.push_back(
                    text(labels[y][x])
                    | bold
                    | color(label_color(labels[y][x]))
                    | center
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

            // text(
            //     "mouse = (" +
            //     std::to_string(mouse_x) + ", " +
            //     std::to_string(mouse_y) + ")"
            // ),

            // text(
            //     "tile = (" +
            //     std::to_string(tile_x) + ", " +
            //     std::to_string(tile_y) + ")"
            // ),
        });
    })),
          menu_button(Button("Main menu", std::move(on_menu))),
          restart_button(Button("Restart", [this] { restart(); })),
          comp(CatchEvent(
              Renderer(
                  Container::Vertical({
                      Container::Horizontal({
                          menu_button,
                          Maybe(restart_button, &finished),
                      }),
                      renderer,
                  }),
                  [this] {
                      const auto status = state == State::Won
                          ? "You win"
                          : state == State::Lost ? "You lose" : "";

                      return vbox({
                          renderer->Render(),
                          hbox({
                              menu_button->Render() | size(WIDTH, EQUAL, 13),
                              finished
                                  ? restart_button->Render() | size(WIDTH, EQUAL, 11)
                                  : emptyElement(),
                              text(status) | vcenter | borderEmpty,
                              text(timer_text()) | vcenter | borderEmpty,
                          }),
                      });
                  }),
              [this](Event event) {
        if (!event.is_mouse())
            return false;

        const Mouse& mouse = event.mouse();

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
            state != State::Playing) {
            return true;
        } else if (tile_x != -1 &&
            mouse.button == Mouse::Right &&
            mouse.motion == Mouse::Pressed &&
            !clicked[tile_y][tile_x]) {
            flagged[tile_y][tile_x] = !flagged[tile_y][tile_x];
        } else if (tile_x != -1 &&
            mouse.button == Mouse::Left &&
            mouse.motion == Mouse::Pressed &&
            !flagged[tile_y][tile_x]) {
            if (!timer_started) {
                timer_started = true;
                timer_start = std::chrono::steady_clock::now();
            }

            auto picked = pick(b, tile_x, tile_y);
            if(picked) {
                for (const auto& [cell, x, y] : picked->picked) {
                    clicked[y][x] = true;
                    labels[y][x] = *cell == 0 ? "" : std::to_string(*cell);
                }

                bool all_safe_cells_revealed = true;
                for (int y = 0; y < ROWS && all_safe_cells_revealed; ++y) {
                    for (int x = 0; x < COLS; ++x) {
                        if (b[y][x] && !clicked[y][x]) {
                            all_safe_cells_revealed = false;
                            break;
                        }
                    }
                }
                if (all_safe_cells_revealed) {
                    finish(State::Won);
                }
            } else {
                for (const auto& [cell, x, y] : picked.error().mines) {
                    clicked[y][x] = true;
                    labels[y][x] = cell ? (*cell == 0 ? "" : std::to_string(*cell)) : "X";
                }
                finish(State::Lost);
            }
        }

        return tile_x != -1;
    })) {}
};

template<class Board>
auto make_view(Board board, std::function<void()> on_menu) -> View<Board> {
    return View<Board>(std::move(board), std::move(on_menu));
}

}