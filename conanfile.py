from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.build import check_max_cppstd, check_min_cppstd

class PPLRecipe(ConanFile):
    name = "vizmo"
    version = "1.1.0"

    # Optional metadata
    license = "BSD 3-Clause License"
    author = "Parasol"
    url = "https://parasollab.web.illinois.edu/resources/"
    description = "A visualization tool for PMPL problems and results."
    topics = ("Motion Planning", "Robotics", "Visualization")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    generators = "CMakeToolchain", "CMakeDeps"

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.20.0]")

    def validate(self):
        check_min_cppstd(self, "11")

    def requirements(self):
        self.requires("cgal/5.5.2")
        self.requires("boost/1.82.0")
        self.requires("catch2/3.3.2")
        self.requires("qt/6.3.2")

        # dependencies for above packages (explicit multi-version conflict resolution)
        self.requires("libpng/1.6.40", override=True)
        self.requires("glib/2.77.0", override=True)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [""]