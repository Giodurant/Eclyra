# ECLYRA - Prototipo tecnico iOS (raylib 5.5 + backend SDL2)
#
# Desktop (Linux, Steam Deck, macOS, Windows) - stessa strada SDL dell'iPhone:
#   cmake -S . -B build && cmake --build build -j
#   ./build/eclyra_proto              (--selftest per la prova automatica)
#
# iOS / iPadOS (su un Mac con Xcode): vedi build_ios.sh e PROTOTIPO_IOS.md
#   cmake -S . -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DECLYRA_TEAM_ID=XXXXXXXXXX
#
# raylib non viene compilata con il suo CMake: si prendono i sorgenti e si compilano qui con
# PLATFORM_DESKTOP_SDL + OpenGL ES 2, cosi' la configurazione e' identica su desktop e su iOS.
cmake_minimum_required(VERSION 3.21)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "iOS minimo")
  project(ECLYRA_PROTO VERSION 0.1.0 LANGUAGES C OBJC)
else()
  project(ECLYRA_PROTO VERSION 0.1.0 LANGUAGES C)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
include(FetchContent)

option(PROTO_USE_SYSTEM_SDL "Usa SDL2 installata nel sistema (desktop)" ON)
set(ECLYRA_TEAM_ID "" CACHE STRING "Apple Developer Team ID (firma su dispositivo)")
set(ECLYRA_BUNDLE_ID "com.eclyra.proto" CACHE STRING "Bundle identifier dell'app di prova")

# ---------------------------------------------------------------- SDL2
set(PROTO_SDL_FROM_SOURCE OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR NOT PROTO_USE_SYSTEM_SDL)
  set(PROTO_SDL_FROM_SOURCE ON)
else()
  find_package(SDL2 QUIET)
  if(NOT SDL2_FOUND)
    set(PROTO_SDL_FROM_SOURCE ON)
  endif()
endif()

if(PROTO_SDL_FROM_SOURCE)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.9
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(SDL2)
  set(PROTO_SDL_LIBS SDL2::SDL2main SDL2::SDL2-static)
else()
  if(TARGET SDL2::SDL2main)
    set(PROTO_SDL_LIBS SDL2::SDL2main SDL2::SDL2)
  else()
    set(PROTO_SDL_LIBS SDL2::SDL2)
  endif()
endif()

# ---------------------------------------------------------------- raylib (solo sorgenti)
# SOURCE_SUBDIR inesistente: scarica i sorgenti senza usare il CMake di raylib
FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR solo_sorgenti)
FetchContent_MakeAvailable(raylib)
set(RL ${raylib_SOURCE_DIR}/src)

add_library(raylib_sdl STATIC
  ${RL}/rcore.c ${RL}/rshapes.c ${RL}/rtextures.c ${RL}/rtext.c ${RL}/raudio.c ${RL}/utils.c)
target_include_directories(raylib_sdl PUBLIC ${RL})
target_compile_definitions(raylib_sdl PUBLIC PLATFORM_DESKTOP_SDL GRAPHICS_API_OPENGL_ES2)
target_link_libraries(raylib_sdl PUBLIC ${PROTO_SDL_LIBS})
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  # miniaudio su iOS usa AVAudioSession: va compilata come Objective-C
  set_source_files_properties(${RL}/raudio.c PROPERTIES COMPILE_FLAGS "-x objective-c")
  target_compile_options(raylib_sdl PRIVATE -Wno-deprecated-declarations)
  target_link_libraries(raylib_sdl PUBLIC
    "-framework UIKit" "-framework OpenGLES" "-framework QuartzCore" "-framework AVFoundation"
    "-framework AudioToolbox" "-framework CoreAudio" "-framework GameController" "-framework CoreMotion"
    "-framework CoreHaptics" "-framework Metal" "-framework CoreGraphics" "-framework Foundation")
elseif(UNIX AND NOT APPLE)
  target_link_libraries(raylib_sdl PUBLIC m dl pthread)
endif()

# ---------------------------------------------------------------- app
set(PROTO_SRC
  src/main.c
  src/proto_audio.c
  src/platform/platform_sdl.c
  src/render/canvas.c
  src/input/input.c)
set(PROTO_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets/pyrin.png)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  list(APPEND PROTO_SRC src/platform/platform_ios.m)
  set(PROTO_STORYBOARD ${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard)
  add_executable(eclyra_proto MACOSX_BUNDLE ${PROTO_SRC} ${PROTO_ASSETS} ${PROTO_STORYBOARD})
  set_source_files_properties(${PROTO_ASSETS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/assets")
  set_source_files_properties(${PROTO_STORYBOARD} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set_target_properties(eclyra_proto PROPERTIES
    OUTPUT_NAME "EclyraProto"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist.in
    MACOSX_BUNDLE_GUI_IDENTIFIER ${ECLYRA_BUNDLE_ID}
    MACOSX_BUNDLE_BUNDLE_NAME "ECLYRA Proto"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${ECLYRA_BUNDLE_ID}
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET}
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${ECLYRA_TEAM_ID}"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
else()
  add_executable(eclyra_proto ${PROTO_SRC})
  # gli asset accanto all'eseguibile, come nel bundle
  add_custom_command(TARGET eclyra_proto POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:eclyra_proto>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROTO_ASSETS} $<TARGET_FILE_DIR:eclyra_proto>/assets/)
endif()

target_link_libraries(eclyra_proto PRIVATE raylib_sdl)
if(NOT MSVC)
  target_compile_options(eclyra_proto PRIVATE -Wall -Wextra)
endif()

enable_testing()
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  add_test(NAME proto_selftest COMMAND eclyra_proto --selftest)
endif()
