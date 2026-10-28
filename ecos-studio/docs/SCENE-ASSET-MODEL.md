# Scene Graph and Asset Registry v0

## Scene graph
A scene is a hierarchy of nodes. Each node has a stable runtime ID, a name, a local transform (position, Euler rotation in radians, and scale), an optional parent, and child IDs. The runtime computes world transforms by multiplying parent and local matrices. Matrices use row-major storage and translation in the final column. Rotation composition uses Z * Y * X Euler order.

The initial graph supports node creation, reparenting, cycle rejection, transform updates, world-matrix calculation, and recursive subtree removal. It is an in-memory runtime model, not yet a serialized scene format or editor viewport.

## Asset registry
Assets have stable string IDs, a type label, a project-relative path, and declared dependencies. Paths are normalized and absolute/path-traversal paths are rejected. Dependency validation reports unresolved references. Asset files are not imported or decoded yet; content hashing, cache invalidation, serialization, and importers remain future work.

## Compatibility
This is an early internal format/API and is not a frozen commercial SDK. Public serialization must add explicit schema versions and migrations before being treated as stable.
