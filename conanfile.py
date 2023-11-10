import os
import json
from conan import ConanFile, tools
from conan.tools.cmake import CMake, cmake_layout

BOOST_COMPONENTS = [
    "atomic",
    "chrono",
    "container",
    "context",
    "contract",
    "coroutine",
    "date_time",
    "exception",
    "fiber",
    "filesystem",
    "graph",
    "graph_parallel",
    "iostreams",
    "json",
    "locale",
    "log",
    "math",
    "mpi",
    "nowide",
    "program_options",
    "python",
    "random",
    "regex",
    "serialization",
    "stacktrace",
    "system",
    "test",
    "thread",
    "timer",
    "type_erasure",
    "wave",
]

USED_BOOST_COMPONENTS = [
    "atomic", # required by filesystem
    "chrono", # required by libqi
    "container", # required by libqi
    "date_time", # required by libqi
    "exception", # required by libqi
    "locale", # required by libqi
    "filesystem",
    "program_options",
    "random", # required by libqi
    "regex", # required by libqi
    "system", # required by filesystem
    "thread", # required by libqi
]

SDK_DIR = "sdk"

class QiLangConan(ConanFile):
    name = "qilang"
    license = "BSD-3-Clause"
    url = "https://github.com/aldebaran/qilang"
    description = "Code generator for strongly-typed client code for the LibQi library"

    requires = [
        "boost/[~1.78]",
        "qi/4.0.1",
    ]

    tool_requires = [
        "bison/[^3.5]",
        "flex/[^2.6]",
    ]

    test_requires = [
        "gtest/cci.20210126",
    ]

    exports = "project.json"
    exports_sources = [
        "CMakeLists.txt",
        "project.json",
        "cmake/*",
        "qilang/*",
        "src/*",
        "tests/*"
    ]

    generators = "CMakeToolchain", "CMakeDeps"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    default_options = {
        "boost/*:shared": True,
    }

    # Disable every components of Boost unless we actively use them.
    default_options.update({
        f"boost/*:without_{_name}": False
        if _name in USED_BOOST_COMPONENTS else True
        for _name in BOOST_COMPONENTS
    })

    def init(self):
        project_cfg_file_path = os.path.join(self.recipe_folder, "project.json")
        project_cfg_data = tools.files.load(self, project_cfg_file_path)
        self.cfg = json.loads(project_cfg_data)

    def set_version(self):
        if self.version:
            return

        version = self.cfg["version"]

        # For development version, try adding a suffix with the revision SHA1.
        revision = None
        if version.endswith('-dev'):
            git = tools.scm.Git(self)
            try:
                revision = git.get_commit()[:8]
                if git.is_dirty():
                    revision += ".dirty"
            # Exception: most likely the recipe folder is not a Git repository (for
            # instance it is a tarball of sources), abort adding the revision.
            except Exception as ex:
                self.output.info("Cannot get version information from git repository, will use a date revision instead"
                                 + f"(exception: {ex})")
                import datetime
                now = datetime.datetime.now()
                revision = now.strftime("%Y%m%d%H%M%S")
        if revision:
            version += f".{revision}"

        self.version = version

    def layout(self):
        # Configure the format of the build folder name, based on the value of some variables.
        self.folders.build_folder_vars = [
            "settings.os",
            "settings.arch",
            "settings.compiler",
            "settings.build_type",
        ]

        # The cmake_layout() sets the folders and cpp attributes to follow the
        # structure of a typical CMake project.
        cmake_layout(self)

    def validate(self):
        # Require at least C++17
        if self.settings.compiler.cppstd:
            tools.build.check_min_cppstd(self, "17")

    def build(self):
        cmake = CMake(self)
        cmake.configure(
            variables={
                "QILANG_VERSION": self.version,
            }
        )
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install(component="runtime")
        cmake.install(component="devel")

    def package_info(self):
        qicc = self.cpp_info.components["qicc"]
        qicc.libs = []
        qicc.requires = [
            "qi::qi",
            "boost::program_options",
            "boost::filesystem",
        ]

        # qilang-tools
        #
        # The builddirs needs to be set on a component, even though it should
        # belong to the entire package. This is because if set on the cpp_info
        # member directly, Conan does not process it. By default, we choose to
        # set it on the "qicc" component.
        #
        # Setting the builddirs enables consumers to find the qilang-tools package
        # configuration file through find_package when using the CMakeToolchain
        # generator (it adds the directories to the CMAKE_PREFIX_PATH).
        #
        # Adding the package_folder to the builddirs does not work when not in
        # the build directory because Conan automatically and silently ignores
        # that value, so we have to set the subdirectory to the qilang-tools
        # package configuration file as the builddir instead.
        qicc.builddirs.append(os.path.join("lib", "cmake", "qilang-tools"))
