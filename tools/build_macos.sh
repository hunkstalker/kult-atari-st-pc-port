#!/bin/sh
# Empaqueta Kult.app, firma ad-hoc (o Developer ID), crea el DMG y un ZIP
# que contiene solo el DMG.
# Uso: KULT_VERSION=1.1.0 tools/build_macos.sh <stage-dir> <out-dir>
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
STAGE=${1:?falta stage-dir}
OUTDIR=${2:?falta out-dir}
VERSION=${KULT_VERSION:-1.1.0}
APP="$STAGE/Kult.app"
EXE="$APP/Contents/MacOS/Kult"
LEEME="$STAGE/LEEME.txt"
SIGN_ID=${MACOS_SIGN_IDENTITY:-}

[ -d "$APP" ] || { echo "ERROR: falta $APP" >&2; exit 2; }
[ -x "$EXE" ] || { echo "ERROR: falta $EXE" >&2; exit 2; }
[ -f "$LEEME" ] || { echo "ERROR: falta $LEEME" >&2; exit 2; }

need() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "ERROR: falta '$1'." >&2; exit 2
    }
}
need lipo
need otool
need codesign
need hdiutil
need shasum
need zip

ARCHS=$(lipo -archs "$EXE")
for arch in arm64 x86_64; do
    case " $ARCHS " in
        *" $arch "*) ;;
        *) echo "ERROR: falta la arquitectura $arch en $EXE ($ARCHS)" >&2; exit 1 ;;
    esac
done

SDL_DYLIB="$APP/Contents/Frameworks/libSDL3.0.dylib"
if [ -f "$SDL_DYLIB" ]; then
    install_name_tool -id @rpath/libSDL3.0.dylib "$SDL_DYLIB"
    dep=$(otool -L "$EXE" | awk '/libSDL3/ {print $1; exit}')
    if [ -n "$dep" ] && [ "$dep" != "@rpath/libSDL3.0.dylib" ]; then
        install_name_tool -change "$dep" @rpath/libSDL3.0.dylib "$EXE"
    fi
fi

if [ -n "$SIGN_ID" ]; then
    if [ -f "$SDL_DYLIB" ]; then
        codesign --force --options runtime --timestamp --sign "$SIGN_ID" "$SDL_DYLIB"
    fi
    codesign --force --options runtime --timestamp --sign "$SIGN_ID" "$APP"
else
    codesign --force --deep --sign - "$APP"
fi
codesign --verify --deep --strict "$APP"

TMP=$(mktemp -d "${TMPDIR:-/tmp}/kult-macos.XXXXXX")
cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT INT TERM
PKG="$TMP/kult-$VERSION-macos"
mkdir -p "$PKG"
cp -R "$APP" "$PKG/"
cp "$LEEME" "$PKG/LEEME.txt"
cp "$STAGE/Kult Settings.txt" "$PKG/Kult Settings.txt"
cp "$STAGE/Kult Settings.png" "$PKG/Kult Settings.png"
cp "$STAGE/LICENSE" "$PKG/LICENSE"
cp "$STAGE/THIRD_PARTY_NOTICES.md" "$PKG/THIRD_PARTY_NOTICES.md"
cp -R "$STAGE/LICENSES" "$PKG/LICENSES"
ln -s /Applications "$PKG/Applications"

mkdir -p "$OUTDIR"
ZIP="$OUTDIR/kult-$VERSION-macos-universal.zip"
DMG="$OUTDIR/kult-$VERSION-macos-universal.dmg"
rm -f "$ZIP" "$DMG"
hdiutil create -volname "Kult $VERSION" -srcfolder "$PKG" -ov -format UDZO "$DMG"

if [ -n "$SIGN_ID" ]; then
    codesign --force --timestamp --sign "$SIGN_ID" "$DMG"
fi

if [ -n "${APPLE_ID:-}" ] && [ -n "${APPLE_TEAM_ID:-}" ] && [ -n "${APPLE_APP_PASSWORD:-}" ] && [ -n "$SIGN_ID" ]; then
    xcrun notarytool submit "$DMG" \
        --apple-id "$APPLE_ID" \
        --team-id "$APPLE_TEAM_ID" \
        --password "$APPLE_APP_PASSWORD" \
        --wait
    xcrun stapler staple "$DMG"
fi

# El ZIP de release contiene solo el DMG, ya firmado/notarizado.
# El DMG se conserva en el artifact de CI para la QA manual.
(cd "$OUTDIR" && zip -q -X -j "$(basename "$ZIP")" "$(basename "$DMG")" && \
    shasum -a 256 "$(basename "$ZIP")" > "$(basename "$ZIP").sha256")
cat "$ZIP.sha256"
echo "OK: $ZIP"
echo "OK: $DMG"
