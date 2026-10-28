# Camera and Projection Math

ECOS Studio provides dependency-free view/projection math using the scene system's row-major matrix storage and column-vector convention.

- `look_at` builds a right-handed view matrix and rejects invalid, coincident, or degenerate camera vectors.
- `perspective` builds an OpenGL-style perspective matrix with NDC depth in [-1, 1].
- `orthographic` builds a matching orthographic projection.
- `project_to_ndc` applies a view-projection matrix and performs homogeneous perspective division with finite-value checks.

The CPU reference renderer has not yet been converted to use this camera API, and no GPU backend or interactive viewport is implied by this math layer. Camera clipping, frustum culling, viewport mapping, and input-driven camera controls are later integrations.
