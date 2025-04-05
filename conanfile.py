from conan import ConanFile
from conan.tools.cmake import CMakeToolchain


class PajladaSettings(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("rapidjson/cci.20200410")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()
