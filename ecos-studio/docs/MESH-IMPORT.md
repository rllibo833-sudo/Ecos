# Mesh Import Pipeline v1

The Wavefront OBJ importer supports:
- Vertex positions (`v x y z`), texture coordinates (`vt u v`), and vertex normals (`vn x y z`).
- Faces with positive or negative position/UV/normal indices, including `v`, `v/vt`, `v//vn`, and `v/vt/vn` forms.
- Polygon fan triangulation and vertex expansion at UV/normal seams so each output index addresses one complete attribute tuple.
- Finite-number and index-range validation, 32-bit output-index capacity checks, and axis-aligned position bounds.

Output `positions`, `texcoords`, and `normals` are aligned one-to-one. Missing UVs and normals are zero-filled; this version does not generate missing normals. Materials, groups, smoothing rules, tangent generation, content hashes, cache metadata, and glTF remain future work. This is a CPU-side importer, not a full production interchange pipeline.
