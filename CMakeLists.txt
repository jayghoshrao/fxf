cmake_minimum_required (VERSION 3.22)

project(fxf
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Fetch Dependencies -------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v6.1.9
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)

FetchContent_Declare(cli11_proj
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG        v2.5.0
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(rapidfuzz
  GIT_REPOSITORY https://github.com/rapidfuzz/rapidfuzz-cpp.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(ftxui cli11_proj rapidfuzz Catch2)

# ------------------------------------------------------------------------------

add_executable(fxf src/utils.cpp src/command.cpp src/registries.cpp src/app.cpp src/main.cpp)
target_include_directories(fxf PRIVATE src)

target_link_libraries(fxf
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE CLI11::CLI11
  PRIVATE rapidfuzz::rapidfuzz
)

if (EMSCRIPTEN)
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")

  foreach(file "index.html" "run_webassembly.py")
    configure_file("src/${file}" ${file})
  endforeach(file)
endif()

# --- Tests --------------------------------------------------------------------
enable_testing()

add_executable(tests
  tests/test_utils.cpp
  tests/test_rowtable.cpp
  src/utils.cpp
)
target_include_directories(tests PRIVATE src)
target_link_libraries(tests
  PRIVATE Catch2::Catch2WithMain
  PRIVATE ftxui::component
  PRIVATE rapidfuzz::rapidfuzz
)

include(CTest)
include(Catch)
catch_discover_tests(tests)
