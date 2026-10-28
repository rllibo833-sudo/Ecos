# Collision Query Foundation

The first physics slice provides validated geometric queries, not a full rigid-body solver.

- Axis-aligned bounding-box overlap and sphere overlap tests.
- Ray-versus-AABB slab intersection and ray-versus-sphere intersection.
- Ray directions are normalized internally, so hit distance is in world units even when the caller provides a non-unit direction.
- Hits return distance, position, and a surface normal; misses are successful query results with hit=false.
- Invalid bounds, non-finite values, negative radii, and zero-length ray directions return typed errors.

Not yet implemented: broad-phase acceleration structures, triangle-mesh collision, continuous collision detection, contact manifolds, rigid-body integration, constraints, character controllers, and physics stepping.
