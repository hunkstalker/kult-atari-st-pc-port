#!/bin/sh
# Construye un AppImage desde el staging de CMake.
# Uso: tools/build_appimage.sh <stage-dir> <salida.AppImage> [appimagetool]
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
STAGE=${1:?falta stage-dir}
OUT=${2:?falta salida.AppImage}
TOOL=${3:-${APPIMAGETOOL:-"$ROOT/third_party/appimagetool-x86_64.AppImage"}}
[ -x "$TOOL" ] || { echo "ERROR: falta appimagetool: $TOOL" >&2; exit 2; }
[ -x "$STAGE/kult" ] || { echo "ERROR: falta $STAGE/kult" >&2; exit 2; }
[ -f "$STAGE/lib/libSDL3.so.0" ] || { echo "ERROR: falta SDL3 en $STAGE/lib" >&2; exit 2; }
[ -f "$STAGE/data/language.data" ] || { echo "ERROR: falta language.data" >&2; exit 2; }

APPDIR=$(mktemp -d "${TMPDIR:-/tmp}/kult-appdir.XXXXXX")
cleanup() { rm -rf "$APPDIR"; }
trap cleanup EXIT INT TERM

mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/kult"
cp "$STAGE/kult" "$APPDIR/usr/bin/kult"
cp "$STAGE/lib/libSDL3.so.0" "$APPDIR/usr/lib/libSDL3.so.0"
cp "$STAGE/data/language.data" "$APPDIR/usr/share/kult/language.data"
cp "$STAGE/LEEME_LINUX.txt" "$APPDIR/usr/share/kult/LEEME_LINUX.txt"
cp "$STAGE/Kult Settings.txt" "$APPDIR/usr/share/kult/Kult Settings.txt"
cp "$STAGE/Kult Settings.png" "$APPDIR/usr/share/kult/Kult Settings.png"
cp "$STAGE/Kult Settings.png" "$APPDIR/kult.png"
cp "$STAGE/LICENSE" "$APPDIR/usr/share/kult/LICENSE"
cp "$STAGE/THIRD_PARTY_NOTICES.md" "$APPDIR/usr/share/kult/THIRD_PARTY_NOTICES.md"
cp -R "$STAGE/LICENSES" "$APPDIR/usr/share/kult/LICENSES"

cat > "$APPDIR/AppRun" <<'APPRUN'
#!/bin/sh
set -eu
APPDIR=${APPDIR:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}
if [ -n "${APPIMAGE:-}" ]; then
    BASE=$(CDPATH= cd -- "$(dirname -- "$APPIMAGE")" && pwd)
else
    BASE=${KULT_BASE_DIR:-$PWD}
fi
mkdir -p "$BASE/data" "$BASE/saves"
cp -f "$APPDIR/usr/share/kult/language.data" "$BASE/data/language.data"
cp -f "$APPDIR/usr/share/kult/LEEME_LINUX.txt" "$BASE/LEEME_LINUX.txt" 2>/dev/null || true
export KULT_BASE_DIR="$BASE"
export LD_LIBRARY_PATH="$APPDIR/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$APPDIR/usr/bin/kult" "$@"
APPRUN
chmod +x "$APPDIR/AppRun"

cat > "$APPDIR/kult.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=Kult
Comment=Native PC port of Kult: The Temple of Flying Saucers
Exec=kult
Icon=kult
Categories=Game;
Terminal=false
DESKTOP

APPIMAGE_EXTRACT_AND_RUN=1 ARCH=x86_64 "$TOOL" "$APPDIR" "$OUT"
echo "OK: $OUT"
