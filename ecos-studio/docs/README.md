# ECOS Studio Technical Documentation

This directory documents implemented behavior and explicit limitations. Product-level status lives in [Studio README](../README.md), architecture in [ARCHITECTURE.md](../ARCHITECTURE.md), and the only engineering roadmap in [ROADMAP.md](../ROADMAP.md).

## Current renderer/editor path

- [Native viewport, LOD, bounds and directional shading](NATIVE-VIEWPORT-LOD.md)
- [Scene graph and asset registry](SCENE-ASSET-MODEL.md)
- [OBJ mesh import](MESH-IMPORT.md)
- [MTL material parsing](MTL-IMPORT.md)
- [World manifest format](WORLD-MANIFEST.md)
- [World runtime and viewport streaming](WORLD-RUNTIME.md)
- [CPU reference renderer](CPU-REFERENCE-RENDERER.md)

## Core runtime

- [Engine frame loop](ENGINE-LOOP.md)
- [Input state](INPUT-STATE.md)
- [Input event model](INPUT-EVENT-MODEL.md)
- [Frame clock](FRAME-CLOCK.md)
- [Camera math](CAMERA-MATH.md)
- [Physics queries](PHYSICS-QUERIES.md)
- [Audio mixer](AUDIO-MIXER.md)
- [Project format](PROJECT-FORMAT.md)
- [Asset content fingerprints](ASSET-CONTENT-HASH.md)

## Documentation policy

When source behavior changes, update the relevant subsystem document and the top-level status/roadmap in the same change. Do not leave superseded claims in active documentation. Keep detailed documents when they remain accurate; archive only genuinely superseded designs, and link archived history to the current canonical document.
