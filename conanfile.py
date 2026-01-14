from conan import ConanFile
from conan.tools.cmake import cmake_layout
import platform

class RTypeConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        if not (self.settings.os == "Macos" and self.settings.arch == "armv8"):
            self.requires("sfml/2.6.1")
        self.requires("boost/1.86.0")
        self.requires("gtest/1.14.0")
        self.requires("lz4/1.9.4")

    def configure(self):
        if self.settings.os == "Macos" and self.settings.arch == "armv8":
            pass
        else:
            self.options["sfml"].graphics = True
            self.options["sfml"].window = True
            if self.settings.os == "Windows":
                self.options["sfml"].audio = False
            else:
                self.options["sfml"].audio = True
            self.options["sfml"].network = True

    def layout(self):
        cmake_layout(self)
