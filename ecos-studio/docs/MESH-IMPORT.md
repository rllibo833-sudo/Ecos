# Mesh Import Pipeline v0

The first mesh importer reads a safe subset of Wavefront OBJ text:
- Vertex positions (`v x y z`).
- Faces with positive or negative vertex indices.
- Polygon fan triangulation into indexed triangles.
- Finite-number validation, index-range validation, and computed axis-aligned bounds.

Texture coordinates, normals, materials, groups, smoothing, and other records are currently ignored. This importer is an initial CPU-side asset-pipeline component, not a GPU renderer or a production-grade replacement for a full interchange pipeline. Future work includes normal/UV support, tangent generation, material slots, content hashes, cache metadata, and glTF support.
