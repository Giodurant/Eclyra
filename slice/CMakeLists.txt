# ECLYRA - Vertical slice visiva (eseguibile separato; il gioco principale non cambia)
#
# La slice compila la logica del gioco (eclyra_core) e i moduli di mondo e lotta del gioco
# senza modificarli, e sostituisce il loro rendering con quello del Design System v2
# (vedi src/legacy_shim.c). Asset generati da tools/design/slice_export.py.

set(SLICE_SRC
  src/main.c src/tour.c src/canvas.c src/azioni.c src/gfx.c src/legacy_shim.c
  src/vista_mondo.c src/vista_dialogo.c src/vista_lotta.c src/vista_evoluzione.c src/vista_titolo.c
  src/pannelli.c src/bestiario.c src/mappa.c)

# moduli del gioco usati cosi' come sono (logica di mondo, NPC, giocatore, flusso della lotta)
set(SLICE_SRC_GIOCO
  ${CMAKE_SOURCE_DIR}/src/world.c ${CMAKE_SOURCE_DIR}/src/player.c ${CMAKE_SOURCE_DIR}/src/npc.c
  ${CMAKE_SOURCE_DIR}/src/battle_ui.c ${CMAKE_SOURCE_DIR}/src/ui_common.c ${CMAKE_SOURCE_DIR}/src/input.c)

add_executable(eclyra_slice ${SLICE_SRC} ${SLICE_SRC_GIOCO})
target_include_directories(eclyra_slice PRIVATE src ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(eclyra_slice PRIVATE eclyra_core raylib)
target_compile_options(eclyra_slice PRIVATE ${ECLYRA_WARN})
if(APPLE)
  target_link_libraries(eclyra_slice PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

if(EMSCRIPTEN)
  # Versione web: pagina index.html + wasm, asset impacchettati in un unico file .data
  set_target_properties(eclyra_slice PROPERTIES OUTPUT_NAME index SUFFIX ".html")
  target_link_options(eclyra_slice PRIVATE
    -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 -sTOTAL_STACK=4194304
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/slice_assets"
    "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html")
  target_compile_options(eclyra_slice PRIVATE -Wno-gnu-zero-variadic-macro-arguments)
  set_property(TARGET eclyra_slice APPEND PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
  add_custom_command(TARGET eclyra_slice POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/static $<TARGET_FILE_DIR:eclyra_slice>)
  return()
endif()

add_custom_command(TARGET eclyra_slice POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:eclyra_slice>/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:eclyra_slice>/slice_assets)
