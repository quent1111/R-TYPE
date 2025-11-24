from conan import ConanFile
from conan.tools.cmake import cmake_layout
import platform

class RTypeConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        # Skip SFML on macOS ARM64 (use Homebrew instead)
        if not (self.settings.os == "Macos" and self.settings.arch == "armv8"):
            self.requires("sfml/2.6.1")
        self.requires("asio/1.30.2")
        self.requires("gtest/1.14.0")

    def configure(self):
        if self.settings.os == "Macos" and self.settings.arch == "armv8":
            # Skip SFML on macOS ARM64
            pass
        else:
            # Configure SFML options for other platforms
            self.options["sfml"].graphics = True
            self.options["sfml"].window = True
            self.options["sfml"].audio = True
            self.options["sfml"].network = False

    def layout(self):
        cmake_layout(self)
