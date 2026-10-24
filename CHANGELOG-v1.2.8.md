# VETER v1.2.8 — launcher and responsive layout fixes

- Fixed the launcher icon lookup by loading `veter-button.png` through `Mod::get()->getResourcesDir()`.
- Added a cyan `V` fallback when the icon file genuinely cannot be loaded.
- Replaced the fixed menu launcher with a touch-aware draggable layer. Dragging moves the button, clamps it inside the viewport and saves its coordinates; tapping it opens VETER.
- Made the main popup dimensions screen-aware and recalculated the content panel, category rail, three columns, button widths and vertical gaps based on actual popup size.
- Updated `mod.json`, `CMakeLists.txt` and the load log to version 1.2.8.

The source archive has been checked for ZIP integrity and JSON validity. It has **not** been built against the Geode SDK here; confirm the Android32/Android64 GitHub Actions jobs before installing.
