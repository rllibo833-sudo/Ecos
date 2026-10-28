# OBJ Mesh Import

ECOS Studio imports a practical subset of Wavefront OBJ into an indexed, expanded-vertex mesh.

Supported:
- positions (`v`), texture coordinates (`vt`), and normals (`vn`)
- faces using `v`, `v/vt`, `v//vn`, and `v/vt/vn` corner forms
- independent position/UV/normal indices, positive and negative indices, and polygon fan triangulation
- vertex expansion at attribute seams and 32-bit index capacity checks
- finite numeric validation, face-reference validation, axis-aligned bounds
- area-weighted generated smooth normals for corners with missing or zero-length normals; explicit non-zero OBJ normals are preserved

Missing texture coordinates are zero-filled. Generated normals smooth across shared expanded vertices; UV seams may therefore produce separate normals at the same geometric position. Polygon fan triangulation assumes convex polygons. Material libraries (`mtllib`/`usemtl`), smoothing-group semantics, tangent generation, robust concave polygon triangulation, and glTF import remain future work.

The CPU reference renderer is deterministic and intended for tests and tooling, not a substitute for a GPU viewport.
