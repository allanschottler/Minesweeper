cmake_minimum_required(VERSION 3.20)

project(Minesweeper LANGUAGES CXX)

find_package(ftxui REQUIRED)

add_executable(minesweeper main.cpp)

target_compile_features(minesweeper PRIVATE cxx_std_23)

target_link_libraries(minesweeper
	PRIVATE
		ftxui::component
		ftxui::dom
		ftxui::screen
)
