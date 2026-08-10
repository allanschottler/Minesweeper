from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class MinesweeperRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "ftxui/7.0.2"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        CMakeDeps(self).generate()
        CMakeToolchain(self).generate()
