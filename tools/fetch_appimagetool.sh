#!/bin/sh
# Descarga appimagetool (AppImageKit) en third_party/.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
THIRD=${THIRD_PARTY_DIR:-"$ROOT/third_party"}
URL=https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
SHA=95cbe7cce9717fce90c484e34052ee7c7f1d7635b33c12525b4776826a7d29b6
OUT="$THIRD/appimagetool-x86_64.AppImage"
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
mkdir -p "$THIRD"
verify() {
    [ -f "$OUT" ] || return 1
    got=$(sha256sum "$OUT" | cut -d' ' -f1)
    [ "$got" = "$SHA" ] || { echo "ERROR: SHA-256 incorrecto para $OUT" >&2; return 1; }
}
if [ "$FORCE" -eq 0 ] && verify; then
    chmod +x "$OUT"
    echo "OK  $OUT (ya existe)"
    exit 0
fi
if [ "$CHECK" -eq 1 ]; then
    echo "ERROR: falta o no coincide $OUT" >&2
    exit 1
fi
tmp="$OUT.part"
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
verify_file() {
    got=$(sha256sum "$1" | cut -d' ' -f1)
    [ "$got" = "$SHA" ] || { echo "ERROR: SHA-256 incorrecto" >&2; return 1; }
}
verify_file "$tmp" || { rm -f "$tmp"; exit 1; }
mv "$tmp" "$OUT"
chmod +x "$OUT"
echo "OK: $OUT"
