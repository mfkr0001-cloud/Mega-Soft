# VETER v1.4.1 — GDH-inspired interface

This is the **independent VETER project**, with its own Geode ID (`dev.chatgpt.gdtools`), metadata, source filenames, features and launcher. It is not a rebranded GDH build.

## UI update
- Dark-purple multi-panel UI with magenta outlines and yellow active highlights.
- Rounded horizontal feature buttons and GDH-style ON/OFF toggle artwork.
- Existing tabs and action callbacks are retained to minimize risk to gameplay features.
- VETER launcher remains draggable and opens/closes the menu.
- Visual assets are stored under `resources/` using VETER-specific filenames.

## Build
Use the included `.github/workflows/main.yml` in GitHub Actions. The workflow builds Android64 and Android32 with the Geode SDK. This archive has been checked for file structure, metadata JSON, source synchronization and ZIP integrity, but has not been compiled with the SDK or tested in-game here.


## v1.4.2 visual update

GDH-inspired pink/purple/navy styling using VETER-owned resource names. This is still the standalone VETER mod (`dev.chatgpt.gdtools`); the current four-tab structure and feature callbacks are retained. Build with GitHub Actions before installing.
