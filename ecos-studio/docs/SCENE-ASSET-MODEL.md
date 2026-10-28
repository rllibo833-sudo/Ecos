# Scene Graph and Asset Registry v1

## Scene graph
A scene is a hierarchy of nodes. Each node has a runtime ID, a name, a local transform (position, Euler rotation in radians, and scale), an optional parent, and child IDs. The runtime computes world transforms by multiplying parent and local matrices. Matrices use row-major storage and translation in the final column. Rotation composition uses Z * Y * X Euler order.

The in-memory graph supports node creation, reparenting, cycle rejection, transform updates, world-matrix calculation, and recursive subtree removal.

## Scene document
Scene text documents use `format_version=1`, a `scene_id`, and node rows in the form `node=id|parent-id-or--|name|position(x,y,z)|rotation(x,y,z)|scale(x,y,z)`. Scene names and IDs cannot contain the pipe delimiter or newlines in version 1. Deserialization validates IDs, numeric transforms, schema version, parent references, and hierarchy cycles. Runtime IDs are regenerated on load while serialized IDs preserve the parent relationship.

Example scene: `examples/hello-world/scenes/main.ecscene`.

## Asset registry
Assets have stable string IDs, a type label, a project-relative path, and declared dependencies. Paths are normalized and absolute/path-traversal paths are rejected. Dependency validation reports unresolved references. Asset files are not imported or decoded yet; content hashing, cache invalidation, serialization, and importers remain future work.

## Compatibility
This is an early internal format/API and is not a frozen commercial SDK. Future format revisions require explicit schema migrations before public compatibility is promised.
