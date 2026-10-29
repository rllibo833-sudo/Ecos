# Native Viewport, GPU Cache, LOD and Directional Shading

**Status:** experimental implementation in `ecos-studio/src/native_editor.cpp`. CI and visual validation must be checked independently for the commit under review.

## Current path

The native GLFW/OpenGL viewport includes an early GPU mesh cache and supports frustum visibility filtering, batching by mesh, instanced drawing when the relevant functions are available, and a non-instanced fallback. This is a compatibility-profile renderer, not a production modern GPU backend.

The viewport can update the world-content runtime from its target position, read loaded cell scene snapshots, offset cell-local transforms by their 1 km cell coordinates, and draw the resident content.

## Mesh bounds and proxy LOD

The mesh importer calculates local-space axis-aligned bounds from imported vertices. Bounds are retained with the mesh and copied into the GPU cache entry. When the viewport selects the distant LOD tier, it builds a box proxy from the source mesh's min/max XYZ values instead of applying a fixed 12 m half-extent. The original instance transform is retained for placement and scaling.

Bounds describe the imported mesh's local AABB; they are not an oriented bounding box, and do not account for runtime deformation. Correct visual sizing depends on source mesh coordinates and instance transforms.

## Flat albedo and simple sun term

The shader applies per-asset `uAlbedo` and a simple directional term:

```glsl
float ndotl = max(dot(normalize(vNormal), normalize(sunDirection)), 0.0);
float light = 0.7 + 0.3 * ndotl;
color = uAlbedo * light;
```

The current implementation uses a fixed directional vector and a simple ambient floor. It is not physically based rendering: no shadow maps, normal maps, texture sampling, exposure, or environment lighting are provided. Correct lighting depends on the validity of mesh normals and the renderer's normal transform.

## Known limitations

- OpenGL compatibility pipeline; no modern backend or production render graph.
- LOD threshold/policy is simple and is not yet driven by screen-space error or asset metadata.
- Proxy uses a local AABB; dynamic meshes and imported coordinate conventions need additional handling.
- No PBR material resource, texture workflow, shadowing, or complete material editor.
- CI compilation does not replace visual testing on representative target hardware.

## Regression invariants

Future changes should preserve the existing cache, culling, instancing/fallback and world-cell integration unless a verified defect requires a focused change. Test and document each such change rather than replacing the whole path.
