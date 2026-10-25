#!/bin/sh
# Descarga y verifica SDL3 3.4.10 para el cross-build de Windows.
# Uso: make deps-win | sh tools/fetch_sdl3.sh [--check|--force]
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DEST=${THIRD_PARTY_DIR:-"$ROOT/third_party"}
VERSION=3.4.10
DEVEL=SDL3-devel-3.4.10-mingw.zip
RUNTIME=SDL3-3.4.10-win32-x64.zip
DEVEL_SHA=5b2bc8589974391e7b3fa4d1bca6c4e09657254afeea57bb4dbe239932188695
RUNTIME_SHA=86e779617c6c2d4e20716e14a5bd9b1210595d62b88a05fed1ccd7fd53c902f9
BASE=https://github.com/libsdl-org/SDL/releases/download/release-$VERSION
CHECK=0
FORCE=0

for arg in "$@"; do
    case "$arg" in
        --check) CHECK=1 ;;
        --force) FORCE=1 ;;
        -h|--help)
            echo "uso: $0 [--check|--force]"
            exit 0 ;;
        *) echo "argumento desconocido: $arg" >&2; exit 2 ;;
    esac
done

need() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "ERROR: falta '$1'. Instala las herramientas necesarias." >&2
        exit 2
    }
}

hash_of() {
    sha256sum "$1" | cut -d' ' -f1
}

verify() {
    [ -f "$1" ] || return 1
    got=$(hash_of "$1")
    [ "$got" = "$2" ] || {
        echo "ERROR: SHA-256 incorrecto para $1" >&2
        echo "  esperado: $2" >&2
        echo "  obtenido: $got" >&2
        return 1
    }
}

fetch() {
    url=$1
    out=$2
    sha=$3
    if [ "$FORCE" -eq 0 ] && verify "$out" "$sha"; then
        echo "OK  $out (ya existe)"
        return 0
    fi
    if [ "$CHECK" -eq 1 ]; then
        echo "ERROR: falta o no coincide $out" >&2
        exit 1
    fi
    tmp="$out.part"
    rm -f "$tmp"
    echo "Descargando $url"
    if command -v wget >/dev/null 2>&1; then
        wget -q -O "$tmp" "$url"
    elif command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 -o "$tmp" "$url"
    else
        echo "ERROR: se necesita wget o curl." >&2
        exit 2
    fi
    verify "$tmp" "$sha" || { rm -f "$tmp"; exit 1; }
    mv "$tmp" "$out"
}

need unzip
need sha256sum
need cut

mkdir -p "$DEST"

fetch "$BASE/$DEVEL" "$DEST/$DEVEL" "$DEVEL_SHA"
fetch "$BASE/$RUNTIME" "$DEST/$RUNTIME" "$RUNTIME_SHA"

DEVEL_DIR="$DEST/SDL3-$VERSION"
RUNTIME_DIR="$DEST/SDL3-$VERSION-win32-x64"
CONFIG="$DEVEL_DIR/x86_64-w64-mingw32/lib/cmake/SDL3/SDL3Config.cmake"
LIB="$DEVEL_DIR/x86_64-w64-mingw32/lib/libSDL3.dll.a"
DLL="$RUNTIME_DIR/SDL3.dll"

extract() {
    zip=$1
    dir=$2
    marker=$3
    if [ "$FORCE" -eq 0 ] && [ -f "$marker" ]; then
        echo "OK  $dir (ya extraído)"
        return 0
    fi
    if [ "$CHECK" -eq 1 ]; then
        echo "ERROR: falta $marker; ejecuta sin --check para extraer." >&2
        exit 1
    fi
    echo "Extrayendo $zip"
    unzip -q -o "$DEST/$zip" -d "$dir"
}

extract "$DEVEL" "$DEST" "$CONFIG"
extract "$RUNTIME" "$RUNTIME_DIR" "$DLL"

for f in "$CONFIG" "$LIB" "$DLL"; do
    [ -f "$f" ] || { echo "ERROR: falta $f" >&2; exit 1; }
done

echo "OK: SDL3 $VERSION listo en $DEST"
