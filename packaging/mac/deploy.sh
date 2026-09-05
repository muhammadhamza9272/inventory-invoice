#!/usr/bin/env bash
#
# Build and package Inventory & Invoice as a macOS .app bundle, then wrap it
# in a .dmg for distribution.
#
# Usage: ./packaging/mac/deploy.sh [build-dir]
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"
APP_NAME="inventory_invoice"
APP_BUNDLE="$BUILD_DIR/$APP_NAME.app"
READABLE_NAME="Inventory & Invoice"
OUT_DIR="$ROOT/dist"
DMG="$OUT_DIR/Inventory-Invoice-macOS.dmg"

QT_BIN="$(brew --prefix qtbase 2>/dev/null)/bin"
MACDEPLOYQT="${QT_BIN:+$QT_BIN/}macdeployqt"

# Homebrew installs Qt modules as separate kegs (qtbase, qtsvg, ...).
# Point extra modules' *_DIR at their own kegs so find_package(Qt6...) resolves.
QT_EXTRA_DIRS=()
svg_dir="$(brew --prefix qtsvg 2>/dev/null)/lib/cmake/Qt6Svg"
[ -n "$svg_dir" ] && QT_EXTRA_DIRS+=("-DQt6Svg_DIR=$svg_dir")

echo "==> Configuring + building (Release)"
cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$(brew --prefix qtbase)/lib/cmake" \
    "${QT_EXTRA_DIRS[@]}" >/dev/null
cmake --build "$BUILD_DIR" --config Release >/dev/null

echo "==> Deploying Qt frameworks into bundle"
"$MACDEPLOYQT" "$APP_BUNDLE"

echo "==> Naming bundle"
BUNDLE_TMP="$BUILD_DIR/$READABLE_NAME.app"
if [ "$APP_BUNDLE" != "$BUNDLE_TMP" ]; then
    rm -rf "$BUNDLE_TMP"
    mv "$APP_BUNDLE" "$BUNDLE_TMP"
fi

echo "==> Creating DMG"
mkdir -p "$OUT_DIR"
rm -f "$DMG"
hdiutil create -volname "$READABLE_NAME" -srcfolder "$BUNDLE_TMP" \
    -ov -format UDZO "$DMG" >/dev/null

echo "==> Done: $DMG"
ls -lh "$DMG"