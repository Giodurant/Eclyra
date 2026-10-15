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
if(EMSCRIPTEN)
  set(CMAKE_C_EXTENSIONS ON)   # EM_ASM (raylib web e pagina) richiede gnu99
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug o Release" FORCE)
endif()

# Strumenti di debug (tasti F, test con screenshot): attivi solo in Debug
option(ECLYRA_DEBUG_TOOLS "Abilita gli strumenti di debug" ON)

# ---------------------------------------------------------------- raylib
# Versione web (emscripten): si compila solo la vertical slice, con raylib per il browser.
#   emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release && cmake --build build-web
if(EMSCRIPTEN)
  set(PLATFORM Web CACHE STRING "" FORCE)
else()
  find_package(raylib 5.0 QUIET)
endif()
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
  src/map.c
  src/events.c
  src/items.c
  src/dialogue.c
  src/partita.c
  src/elyon.c
  src/item_use.c
  src/battle.c
  src/data/items_data.c
  src/data/types.c
  src/data/species_data.c
  src/data/moves_data.c
  src/data/abilities_data.c
  src/data/evolution_data.c
  src/data/trainers_data.c
  src/data/encounters_data.c
)
target_include_directories(eclyra_core PUBLIC src)
target_compile_options(eclyra_core PRIVATE ${ECLYRA_WARN})
if(NOT MSVC)
  target_link_libraries(eclyra_core PUBLIC m)
endif()

if(EMSCRIPTEN)
  add_subdirectory(slice)
  return()
endif()

# ---------------------------------------------------------------- gioco
add_executable(eclyra
  src/main.c
  src/game.c
  src/graphics.c
  src/input.c
  src/menu.c
  src/map_render.c
  src/npc.c
  src/dialogue_ui.c
  src/player.c
  src/world.c
  src/ui_common.c
  src/battle_ui.c
  src/schermate.c
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

# ---------------------------------------------------------------- vertical slice visiva
option(ECLYRA_BUILD_SLICE "Compila la vertical slice visiva (slice/)" ON)
if(ECLYRA_BUILD_SLICE)
  add_subdirectory(slice)
endif()

# ---------------------------------------------------------------- test
enable_testing()
add_executable(test_utils tests/test_utils.c)
target_link_libraries(test_utils PRIVATE eclyra_core)
target_compile_options(test_utils PRIVATE ${ECLYRA_WARN})
add_test(NAME utils COMMAND test_utils)

add_executable(test_mappe tests/test_mappe.c)
target_link_libraries(test_mappe PRIVATE eclyra_core)
target_compile_options(test_mappe PRIVATE ${ECLYRA_WARN})
add_test(NAME mappe COMMAND test_mappe ${CMAKE_SOURCE_DIR}/assets)

add_executable(test_dialoghi tests/test_dialoghi.c)
target_link_libraries(test_dialoghi PRIVATE eclyra_core)
target_compile_options(test_dialoghi PRIVATE ${ECLYRA_WARN})
add_test(NAME dialoghi COMMAND test_dialoghi ${CMAKE_SOURCE_DIR}/assets)

add_executable(test_database tests/test_database.c)
target_link_libraries(test_database PRIVATE eclyra_core)
target_compile_options(test_database PRIVATE ${ECLYRA_WARN})
add_test(NAME database COMMAND test_database)

add_executable(test_elyon tests/test_elyon.c)
target_link_libraries(test_elyon PRIVATE eclyra_core)
target_compile_options(test_elyon PRIVATE ${ECLYRA_WARN})
add_test(NAME elyon COMMAND test_elyon)

add_executable(test_battle tests/test_battle.c)
target_link_libraries(test_battle PRIVATE eclyra_core)
target_compile_options(test_battle PRIVATE ${ECLYRA_WARN})
add_test(NAME battle COMMAND test_battle)

add_executable(test_bilanciamento tests/test_bilanciamento.c)
target_link_libraries(test_bilanciamento PRIVATE eclyra_core)
target_compile_options(test_bilanciamento PRIVATE ${ECLYRA_WARN})
add_test(NAME bilanciamento COMMAND test_bilanciamento 2000)
