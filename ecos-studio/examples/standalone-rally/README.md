# ECOS Rally — standalone sample game

This is a small, playable desktop game shipped as a separate executable by ECOS Studio. It is intentionally a runtime example, not a mock editor screen.

## Build

From the repository root:

```sh
cmake -S ecos-studio -B build/ecos-studio -DCMAKE_BUILD_TYPE=Release
cmake --build build/ecos-studio --target ecos-studio-game --config Release
```

Run `build/ecos-studio/ecos-studio-game` on Linux/macOS, or
`build\\ecos-studio\\Release\\ecos-studio-game.exe` on Windows.

## Play

- Move the left paddle with **W/S** or **Up/Down**.
- The opposing paddle tracks the ball.
- First side to five points wins.
- Press **Space** after the match to restart; press **Escape** to quit.

The standalone runtime uses ECOS Studio's fixed-step engine loop, input snapshots,
generation-safe entity registry, and AABB collision primitive. It opens its own
GLFW/OpenGL window and does not launch or depend on the Studio editor.

CI also launches the executable in a hidden virtual display for a 120-frame
window/render smoke check. This checks executable startup and frame presentation;
it is not a substitute for interactive playtesting on physical hardware.
