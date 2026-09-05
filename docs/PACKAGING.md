# Packaging

In Inventory & Invoice there are two supported targets: **macOS** (the primary
platform) and **Windows** (secondary, for the future office PC).

## macOS

The CMake build already produces a real `.app` bundle: `qt_add_executable` is
called with the `MACOSX_BUNDLE` flag and the bundle gets an Info.plist (name,
`com.muhammadhamza9272.inventory-invoice` bundle id, version 0.1.0).

### One-shot script

```bash
./packaging/mac/deploy.sh
```

What it does:

1. Configures a **Release** build. Homebrew installs every Qt module as its own
   keg, so it passes `CMAKE_PREFIX_PATH=$(brew --prefix qtbase)/lib/cmake`
   together with an explicit `-DQt6Svg_DIR=$(brew --prefix qtsvg)/lib/cmake/Qt6Svg`
   (without that, `find_package(Qt6 ... Svg)` fails — see Gotchas).
2. Builds.
3. Runs `macdeployqt` to copy the Qt frameworks and platform/image-format
   plugins into the bundle.
4. Renames `build/inventory_invoice.app` to `build/Inventory & Invoice.app`.
5. Wraps it in a compressed DMG at `dist/Inventory-Invoice-macOS.dmg`.

The resulting DMG is fully self-contained; you can copy the app into
`/Applications`.

### Manual steps

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$(brew --prefix qtbase)/lib/cmake" \
    -DQt6Svg_DIR="$(brew --prefix qtsvg)/lib/cmake/Qt6Svg"
cmake --build build --config Release
$(brew --prefix qtbase)/bin/macdeployqt "build/Inventory & Invoice.app"
hdiutil create -volname "Inventory & Invoice" -srcfolder \
    "build/Inventory & Invoice.app" -ov -format UDZO \
    dist/Inventory-Invoice-macOS.dmg
```

### Gotchas

- **`-DCMAKE_PREFIX_PATH` alone does not find sibling Homebrew Qt modules.**
  Each formula installs CMake configs under its own keg
  (`qtbase/lib/cmake/Qt6`, `qtsvg/lib/cmake/Qt6Svg`, ...). When a component is
  missing, configure fails with "Failed to find required Qt component". Pass an
  explicit `-DQt6<Module>_DIR` per extra module.
- `macdeployqt` prints `Cannot resolve rpath "@rpath/QtSvg.framework/..."` if the
  app links QtWidgets without the QtSvg library — Widgets loads SVG ICON support
  dynamically and the framework can't be found at deploy time. Adding
  `Qt6::Svg` to `target_link_libraries` removes the warnings and makes the
  bundle complete.
- Code signing / notarization is intentionally not configured (personal /
  portfolio distribution). macOS will show Gatekeeper prompts; this is expected
  until the app is notarized.
- App icon: not yet set (bundle uses the generic executable icon).

## Windows (future office PC)

Build once with Visual Studio or MinGW (Qt 6.x online installer / MSYS2):

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
windeployqt build/Release/inventory_invoice.exe
```

Because the CMakeLists uses `if(APPLE)` only for the MACOSX_BUNDLE flag and
`install(TARGETS ... RUNTIME DESTINATION bin)`, the same sources produce a plain
`inventory_invoice.exe` on Windows.

Suggested installer: **Inno Setup** (free, single-file installers). Steps:

1. `windeployqt` the exe into a staging folder.
2. Point Inno Setup at that folder; install to `{app}`; add start-menu shortcut.
3. Create a code-signing cert when distributing beyond a portfolio demo.

(NSIS is an equally good alternative; Inno Setup is slightly easier to script.)

## Demo / screenshot mode

The app is fully demoable out of the box:

```bash
# Seed sample data, open the Reports tab
./build/Inventory & Invoice.app/Contents/MacOS/inventory_invoice --demo --tab=3
# Seed + immediately open the New Invoice editor
... --demo --new-invoice
```

`--tab=<0..3>` selects Inventory / Invoices / Customers / Reports.