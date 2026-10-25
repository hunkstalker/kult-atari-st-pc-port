#!/bin/sh
# Descarga, compila e instala SDL3 3.4.10 universal para macOS.
# Uso: make deps-macos | sh tools/fetch_sdl3_macos.sh [--check|--force]
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
THIRD=${THIRD_PARTY_DIR:-"$ROOT/third_party"}
VERSION=3.4.10
ARCHIVE=SDL3-$VERSION.tar.gz
URL=https://github.com/libsdl-org/SDL/releases/download/release-$VERSION/$ARCHIVE
SHA=12b34280415ec8418c864408b93d008a20a6530687ee613d60bfbd20411f2785
SRC="$THIRD/SDL3-$VERSION-src-macos"
BUILD="$THIRD/SDL3-$VERSION-macos-build"
PREFIX="$THIRD/sdl3-macos"
ARCHS=${MACOS_ARCHS:-"arm64;x86_64"}
DEPLOY=${MACOS_DEPLOYMENT_TARGET:-11.0}
CHECK=0
FORCE=0

for arg in "$@"; do
    case "$arg" in
        --check) CHECK=1 ;;
        --force) FORCE=1 ;;
        -h|--help) echo "uso: $0 [--check|--force]"; exit 0 ;;
        *) echo "argumento desconocido: $arg" >&2; exit 2 ;;
    esac
done

need() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "ERROR: falta '$1'." >&2; exit 2
    }
}

hash_file() {
    if command -v shasum >/dev/null 2>&1; then
        shasum -a 256 "$1" | cut -d' ' -f1
    else
        sha256sum "$1" | cut -d' ' -f1
    fi
}

verify() {
    [ -f "$1" ] || return 1
    got=$(hash_file "$1")
    [ "$got" = "$2" ] || {
        echo "ERROR: SHA-256 incorrecto para $1" >&2
        echo "  esperado: $2" >&2
        echo "  obtenido: $got" >&2
        return 1
    }
}

need cmake
need tar
mkdir -p "$THIRD"

if [ "$FORCE" -eq 0 ] && [ -f "$PREFIX/lib/libSDL3.0.dylib" ]; then
    echo "OK  $PREFIX (ya compilado)"
    [ "$CHECK" -eq 1 ] && exit 0
else
    if [ "$CHECK" -eq 1 ]; then
        echo "ERROR: falta $PREFIX/lib/libSDL3.0.dylib" >&2
        exit 1
    fi
    if [ "$FORCE" -eq 1 ] || ! verify "$THIRD/$ARCHIVE" "$SHA"; then
        tmp="$THIRD/$ARCHIVE.part"
        rm -f "$tmp"
        echo "Descargando $URL"
        if command -v wget >/dev/null 2>&1; then
            wget -q -O "$tmp" "$URL"
        elif command -v curl >/dev/null 2>&1; then
            curl -fL --retry 3 -o "$tmp" "$URL"
        else
            echo "ERROR: se necesita wget o curl." >&2
            exit 2
        fi
        verify "$tmp" "$SHA" || { rm -f "$tmp"; exit 1; }
        mv "$tmp" "$THIRD/$ARCHIVE"
    else
        echo "OK  $THIRD/$ARCHIVE (ya existe)"
    fi
    rm -rf "$SRC" "$BUILD" "$PREFIX"
    mkdir -p "$SRC"
    tar -xzf "$THIRD/$ARCHIVE" -C "$SRC" --strip-components=1
    cmake -S "$SRC" -B "$BUILD" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_OSX_ARCHITECTURES="$ARCHS" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$DEPLOY" \
        -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_FRAMEWORK=OFF \
        -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF -DSDL_INSTALL_TESTS=OFF
    cmake --build "$BUILD" -j"${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 2)}"
    cmake --install "$BUILD"
fi

for f in "$PREFIX/lib/libSDL3.0.dylib" "$PREFIX/lib/cmake/SDL3/SDL3Config.cmake"; do
    [ -f "$f" ] || { echo "ERROR: falta $f" >&2; exit 1; }
done
echo "OK: SDL3 $VERSION macOS $ARCHS listo en $PREFIX"
