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

    enum class MenuChoice { None, Beginner, Intermediate, Expert, Quit };
    bool running = true;

    while (running) {
        MenuChoice choice = MenuChoice::None;
        auto leave_menu = screen.ExitLoopClosure();

        auto choose = [&](MenuChoice selected) {
            choice = selected;
            leave_menu();
        };

        auto beginner_button = Button("Beginner", [&] { choose(MenuChoice::Beginner); });
        auto intermediate_button = Button("Intermediate", [&] { choose(MenuChoice::Intermediate); });
        auto expert_button = Button("Expert", [&] { choose(MenuChoice::Expert); });
        auto quit_button = Button("Quit", [&] { choose(MenuChoice::Quit); });
        auto buttons = Container::Vertical({
            beginner_button,
            intermediate_button,
            expert_button,
            quit_button,
        });
        auto menu = Renderer(
            buttons,
            [&] {
                return vbox({
                    text("Minesweeper") | bold,
                    beginner_button->Render(),
                    intermediate_button->Render(),
                    expert_button->Render(),
                    quit_button->Render(),
                }) | size(WIDTH, EQUAL, 16);
            });

        screen.Loop(menu);

        if (choice == MenuChoice::Quit || choice == MenuChoice::None) {
            running = false;
            continue;
        }

        auto play = [&]<class Difficulty>() {
            auto view = make_view(
                make_board<Difficulty>(),
                screen.ExitLoopClosure());
            screen.Loop(view.comp);
        };

        switch (choice) {
            case MenuChoice::Beginner: play.template operator()<Beginner>(); break;
            case MenuChoice::Intermediate: play.template operator()<Intermediate>(); break;
            case MenuChoice::Expert: play.template operator()<Expert>(); break;
            case MenuChoice::None:
            case MenuChoice::Quit: break;
        }
    }
}