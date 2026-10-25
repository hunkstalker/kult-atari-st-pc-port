# CMake del ejecutable del port (M7/T-070, macOS T-136). El core no se toca:
# este build enlaza lib/kult-core/src (juego) + boot/ + platform/ + app/, con
# `platform/include` el primero para sombrear os.h/hw.h/psg.h/sched.h con la HAL
# de PC. El Makefile sigue siendo el build del core y del diferencial (`make test`).
#
#   Linux:  cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release ...
#   Windows (cross MinGW): ver cmake/mingw-w64.cmake y docs/BUILD.md.
#   macOS Universal 2: ver docs/BUILD.md y tools/build_macos.sh.
#
# Targets: `kult` (headless) y `kult-sdl` (ventana/audio SDL3). En Windows,
# `kult-sdl` sale como kult.exe y el headless como kult-headless.exe. En macOS,
# `kult-sdl` produce Kult.app.

cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
endif()
project(kult VERSION 1.1.0 LANGUAGES C)

# Ruta a la SDL3 compartida que se empaqueta en Linux/macOS (se puede pasar con
# -DKULT_SDL3_LIB=...). En Windows se usa third_party/SDL3-.../SDL3.dll.
set(KULT_SDL3_LIB "" CACHE FILEPATH "SDL3 shared library to bundle on Linux/macOS")

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
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/mac_dialog.c
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
if(APPLE)
  target_sources(kult-sdl PRIVATE platform/mac_dialog.c)
endif()

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
if(APPLE)
  set_target_properties(kult-sdl PROPERTIES
    OUTPUT_NAME "Kult"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "Kult"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.kultport.kult"
    MACOSX_BUNDLE_ICON_FILE "kult.icns"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "MIT"
    INSTALL_RPATH "@executable_path/../Frameworks")
  if(KULT_SDL3_LIB)
    set(KULT_SDL3_RUNTIME_LIB "${KULT_SDL3_LIB}")
  else()
    find_library(KULT_SDL3_RUNTIME_LIB NAMES SDL3 libSDL3.0.dylib)
  endif()
  if(NOT KULT_SDL3_RUNTIME_LIB OR NOT EXISTS "${KULT_SDL3_RUNTIME_LIB}")
    message(FATAL_ERROR "macOS: define -DKULT_SDL3_LIB=<ruta a libSDL3.0.dylib>")
  endif()
  get_filename_component(KULT_SDL3_RUNTIME_REAL "${KULT_SDL3_RUNTIME_LIB}" REALPATH)
endif()

if(APPLE)
  install(TARGETS kult-sdl BUNDLE DESTINATION .)
else()
  install(TARGETS kult-sdl RUNTIME DESTINATION .)
endif()
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kult Settings.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kult Settings.png"
  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  ${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md
  DESTINATION .)
if(NOT APPLE)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/language.data DESTINATION data)
endif()
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

if(APPLE)
  install(FILES "${KULT_SDL3_RUNTIME_REAL}"
          DESTINATION "Kult.app/Contents/Frameworks" RENAME "libSDL3.0.dylib")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/language.data
          DESTINATION "Kult.app/Contents/Resources/data")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/LEEME_MAC.txt
          DESTINATION "Kult.app/Contents/Resources" RENAME "LEEME.txt")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/kult.icns
          DESTINATION "Kult.app/Contents/Resources")
  install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kult Settings.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/Kult Settings.png"
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md
    DESTINATION "Kult.app/Contents/Resources")
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LICENSES
          DESTINATION "Kult.app/Contents/Resources")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/LEEME_MAC.txt
          DESTINATION . RENAME "LEEME.txt")
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
elseif(APPLE)
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_PACKAGE_FILE_NAME "kult-${PROJECT_VERSION}-macos-universal")
else()
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_PACKAGE_FILE_NAME "kult-${PROJECT_VERSION}-linux-x86_64")
endif()
include(CPack)
