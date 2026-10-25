# CMake del ejecutable del port (M7/T-070). El core no se toca: este build
# enlaza lib/kult-core/src (juego) + boot/ + platform/ + app/, con `platform/include` el
# primero para sombrear os.h/hw.h/psg.h/sched.h con la HAL de PC. El Makefile
# sigue siendo el build del core y del diferencial (`make test`).
#
#   Linux:  cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
#           cmake --build build-cmake -j
#   Windows (cross MinGW):
#           cmake -S . -B build-win -DCMAKE_BUILD_TYPE=Release \
#             -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
#             -DCMAKE_PREFIX_PATH=$PWD/third_party/SDL3-3.4.10/x86_64-w64-mingw32
#           cmake --build build-win -j && (cd build-win && cpack -G ZIP)
#
# Targets: `kult` (headless) y `kult-sdl` (ventana/audio SDL3). En Windows,
# `kult-sdl` sale como kult.exe (el juego) y el headless como kult-headless.exe.

cmake_minimum_required(VERSION 3.16)
project(kult VERSION 1.0.0 LANGUAGES C)

# Ruta a la SDL3 compartida que se empaqueta en Linux (se puede pasar con
# -DKULT_SDL3_LIB=...). En Windows se usa third_party/SDL3-.../SDL3.dll.
set(KULT_SDL3_LIB "" CACHE FILEPATH "SDL3 shared library to bundle on Linux")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---- fuentes (espejo de PORT_SRC del Makefile) ------------------------------
file(GLOB KULT_CORE_SRC     CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/kult-core/src/*.c)
file(GLOB KULT_BOOT_SRC     CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/boot/*.c)
file(GLOB KULT_PLATFORM_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/platform/*.c)
# La entrada/salida SDL (ventana/audio) sólo va al target `kult-sdl`; la copia
# de traza T-073c no se compila en el build estándar. Con KULT_DIAG_TRACE=ON
# se sustituye lib/kult-core/src/dispatch.c por platform/dispatch_trace.c: escribe Y/V de
# todo el guion (sin depender de F12) para diagnosticar una run completa.
option(KULT_DIAG_TRACE "Usar dispatch_trace.c (traza Y/V completa)" OFF)
list(REMOVE_ITEM KULT_PLATFORM_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/video_sdl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/audio_sdl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/dispatch_trace.c)
if(KULT_DIAG_TRACE)
  list(REMOVE_ITEM KULT_CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/lib/kult-core/src/dispatch.c)
  list(APPEND KULT_PLATFORM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platform/dispatch_trace.c)
endif()

# T-073b: solo el core se instrumenta para cobrar el coste medido por función.
# El diferencial (make test) no usa este CMake y sigue sin instrumentar.
set_source_files_properties(${KULT_CORE_SRC}
  PROPERTIES COMPILE_OPTIONS "-finstrument-functions")

add_library(kultport STATIC ${KULT_CORE_SRC} ${KULT_BOOT_SRC} ${KULT_PLATFORM_SRC})
# `platform/include` PRIMERO: sombrea os.h/hw.h/psg.h/sched.h del kit.
target_include_directories(kultport PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/include
  ${CMAKE_CURRENT_SOURCE_DIR}/platform
  ${CMAKE_CURRENT_SOURCE_DIR}/boot
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/kult-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/kult-core/tools/recreate_kit/include)
target_compile_options(kultport PRIVATE -O2 -Wall -Wextra)
target_compile_definitions(kultport PUBLIC KULT_VERSION="${PROJECT_VERSION}")
if(WIN32)
  # platform/log.c usa GetConsoleWindow/MessageBoxA (diagnóstico visible).
  target_link_libraries(kultport PUBLIC user32)
endif()

# ---- ejecutables ------------------------------------------------------------
add_executable(kult app/main.c)
target_link_libraries(kult PRIVATE kultport)

add_executable(kult-sdl app/main.c platform/video_sdl.c platform/audio_sdl.c)
target_compile_definitions(kult-sdl PRIVATE HAVE_SDL3)
target_include_directories(kult-sdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# SDL3: primero el paquete de CMake; si no está (Linux mínimo), pkg-config.
find_package(SDL3 CONFIG QUIET)
if(SDL3_FOUND)
  target_link_libraries(kult-sdl PRIVATE kultport SDL3::SDL3)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
  target_link_libraries(kult-sdl PRIVATE kultport PkgConfig::SDL3)
endif()

# ---- icono de la app (recurso del .exe en Windows) --------------------------
if(WIN32)
  set(KULT_ICON_RC ${CMAKE_CURRENT_SOURCE_DIR}/assets/kult_icon.rc)
  set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -I\"${CMAKE_CURRENT_SOURCE_DIR}/assets\"")
  # windres referencia el .ico por ruta; sin esto, regenerarlo no recompila el
  # recurso y el .exe se queda con el icono anterior.
  set_property(SOURCE ${KULT_ICON_RC} PROPERTY OBJECT_DEPENDS
               ${CMAKE_CURRENT_SOURCE_DIR}/assets/kult_icon.ico)
  target_sources(kult-sdl PRIVATE ${KULT_ICON_RC})
  set_target_properties(kult-sdl PROPERTIES WIN32_EXECUTABLE TRUE OUTPUT_NAME kult)
  set_target_properties(kult PROPERTIES OUTPUT_NAME kult-headless)
endif()

# ---- empaquetado de release -------------------------------------------------
# El paquete NO lleva assets del juego: el usuario aporta sus STX en data/.
# kult-headless sigue siendo un target de desarrollo, pero no se empaqueta.
install(TARGETS kult-sdl RUNTIME DESTINATION .)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kult Settings.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kult Settings.png"
  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  ${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md
  DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/language.data DESTINATION data)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LICENSES DESTINATION .)
if(WIN32)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/LEEME.txt DESTINATION .)
  set(KULT_SDL3_DLL
      ${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3-3.4.10-win32-x64/SDL3.dll)
  if(EXISTS ${KULT_SDL3_DLL})
    install(FILES ${KULT_SDL3_DLL} DESTINATION .)
  else()
    message(WARNING "falta ${KULT_SDL3_DLL}: el paquete no incluirá SDL3.dll")
  endif()
  install(DIRECTORY DESTINATION data)
  install(DIRECTORY DESTINATION saves)
endif()

if(UNIX AND NOT APPLE)
  install(DIRECTORY DESTINATION data)
  install(DIRECTORY DESTINATION saves)
  set_target_properties(kult PROPERTIES OUTPUT_NAME kult-headless)
  set_target_properties(kult-sdl PROPERTIES OUTPUT_NAME kult)
  if(KULT_SDL3_LIB)
    set(KULT_SDL3_RUNTIME_LIB "${KULT_SDL3_LIB}")
  else()
    find_library(KULT_SDL3_RUNTIME_LIB NAMES SDL3 libSDL3.so.0)
  endif()
  if(NOT KULT_SDL3_RUNTIME_LIB OR NOT EXISTS "${KULT_SDL3_RUNTIME_LIB}")
    message(FATAL_ERROR "Linux: define -DKULT_SDL3_LIB=<ruta a libSDL3.so.0>")
  endif()
  get_filename_component(KULT_SDL3_RUNTIME_REAL "${KULT_SDL3_RUNTIME_LIB}" REALPATH)
  install(FILES "${KULT_SDL3_RUNTIME_REAL}" DESTINATION lib RENAME "libSDL3.so.0")
  set_target_properties(kult-sdl PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/LEEME_LINUX.txt
          DESTINATION . RENAME LEEME.txt)
endif()

set(CPACK_PACKAGE_NAME "kult")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Kult Port")
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_PACKAGE_FILE_NAME "kult-${PROJECT_VERSION}-win64")
else()
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_PACKAGE_FILE_NAME "kult-${PROJECT_VERSION}-linux-x86_64")
endif()
include(CPack)
