# ECOS Offline 3D Prototype

Godot 4 project for the first playable, offline ECOS prototype.

## Run
1. Open this folder (game/) in Godot 4.3 or newer.
2. Press F6/F5 or run the project.
3. The ECOS Studio launch screen opens; select JALANKAN GAME OFFLINE.

## Current implemented slice
- Studio-style launch screen that opens the playable scene.
- Procedurally constructed 3D blockout, sky, directional lighting, ground, buildings, and trees.
- CharacterBody3D player with collision, gravity, jumping, WASD movement, mouse look, and cursor release.
- All geometry and gameplay are generated locally; no network/API is required.

## Honest limitations
- This is a first code prototype, not a finished studio or a photorealistic game.
- Environment landmarks are temporary blockout and do not establish new ECOS canon or exact geography.
- Audio, Android export presets, visual assets, device performance, and exported APK still require implementation/validation.
- This repository change has not been run in the Godot editor or built into an APK in this environment.
