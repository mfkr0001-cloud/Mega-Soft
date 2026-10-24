# VETER v1.3.0 — UI sizing and layout fixes

## Changes
- Added a saved button-size option (70%, 80%, 90%, 100%) in Settings.
- Added a saved menu-size option (75%, 85%, 95%, 100%); the popup reopens at the selected size.
- Added a saved button-only opacity option (50%, 65%, 80%, 90%, 100%); window/panel backgrounds remain opaque.
- Reworked the main popup dimensions and button width calculations to keep controls within their columns on narrower screens.
- Made button labels fit both card width and height, tightened Settings rows and repositioned tab cards to prevent overflow.
- The selected tab is saved so changing a UI preference returns to the same tab.
- Preserved the existing feature callbacks and mod ID `dev.chatgpt.gdtools`.

## Build status
This archive contains source files for GitHub Actions, not a prebuilt `.geode`. Android compilation and in-game behavior have not been run in this environment.
