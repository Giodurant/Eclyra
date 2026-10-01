# ECLYRA - Le Cronache dell'Equilibrio
#
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build -j
#   ./build/eclyra            (su Windows: build\Debug\eclyra.exe)
#   ctest --test-dir build    test automatici
#
# raylib: se e' installata nel sistema (find_package) viene usata, altrimenti
# CMake la scarica da GitHub (versione 5.5) alla prima configurazione.
cmake_minimum_required(VERSION 3.16)
project(ECLYRA VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug o Release" FORCE)
endif()

# Strumenti di debug (tasti F, test con screenshot): attivi solo in Debug
option(ECLYRA_DEBUG_TOOLS "Abilita gli strumenti di debug" ON)

# ---------------------------------------------------------------- raylib
find_package(raylib 5.0 QUIET)
if(NOT raylib_FOUND)
  include(FetchContent)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(raylib)
endif()

# ---------------------------------------------------------------- avvisi
if(MSVC)
  set(ECLYRA_WARN /W4)
else()
  set(ECLYRA_WARN -Wall -Wextra -Wpedantic -Wshadow)
endif()

# Logica pura (senza raylib): compilata a parte per poterla testare
add_library(eclyra_core STATIC
  src/utils.c
)
target_include_directories(eclyra_core PUBLIC src)
target_compile_options(eclyra_core PRIVATE ${ECLYRA_WARN})
if(NOT MSVC)
  target_link_libraries(eclyra_core PUBLIC m)
endif()

# ---------------------------------------------------------------- gioco
add_executable(eclyra
  src/main.c
  src/game.c
  src/graphics.c
  src/input.c
  src/menu.c
  src/map.c
  src/player.c
  src/world.c
)
target_link_libraries(eclyra PRIVATE eclyra_core raylib)
target_compile_options(eclyra PRIVATE ${ECLYRA_WARN})
target_compile_definitions(eclyra PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<BOOL:${ECLYRA_DEBUG_TOOLS}>>:ECLYRA_DEBUG>)
if(APPLE)
  target_link_libraries(eclyra PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

# Gli asset vengono copiati accanto all'eseguibile
add_custom_command(TARGET eclyra POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:eclyra>/assets)

# ---------------------------------------------------------------- test
enable_testing()
add_executable(test_utils tests/test_utils.c)
target_link_libraries(test_utils PRIVATE eclyra_core)
target_compile_options(test_utils PRIVATE ${ECLYRA_WARN})
add_test(NAME utils COMMAND test_utils)
