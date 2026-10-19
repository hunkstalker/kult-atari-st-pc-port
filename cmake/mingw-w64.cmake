# Toolchain de cross-compilacion a Windows (x86_64) con MinGW-w64.
# Uso:
#   cmake -S . -B build-win -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
#     -DCMAKE_PREFIX_PATH=$PWD/third_party/SDL3-3.4.10/x86_64-w64-mingw32
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER  x86_64-w64-mingw32-gcc)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
