#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "view.hpp"
#include "game.hpp"

using namespace ms;
using namespace ftxui;

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    screen.TrackMouse(true);

    auto view = make_view(make_board<Expert>());
    screen.Loop(view.comp);
}