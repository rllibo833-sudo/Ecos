# Scene Graph and Asset Registry

## Scene graph

A scene is a hierarchy of nodes with runtime IDs, names, local transforms, optional parents, and child IDs. The runtime computes world transforms from parent/local transforms. Matrices use row-major storage and translation in the final column; Euler composition uses Z * Y * X order.

The graph supports node creation, reparenting, cycle rejection, transform updates, world-matrix calculation, and recursive subtree removal. The versioned `.ecscene` document is used by the native editor scene workflow; serialized IDs preserve hierarchy while runtime IDs may be regenerated on load.

## Asset registry and import

Assets have stable string IDs, type labels, project-relative paths, and declared dependencies. Unsafe paths are rejected. The registry validates references/dependencies and uses SHA-256 content fingerprints where applicable.

OBJ import reads positions, UVs, normals, faces, triangulates polygons, expands attribute seams, generates missing normals, validates finite values and indices, and calculates axis-aligned local-space mesh bounds. Imported project assets use versioned `.ecasset` metadata and integrity validation. The native viewport resolves scene mesh IDs through the validated imported-asset registry.

The GPU renderer cache stores GPU buffers and the mesh bounds used by the native viewport path. Distant LOD proxies derive their local box dimensions from those mesh bounds. This does not yet imply a general-purpose asset cache invalidation/reimport service.

## Format compatibility

These are early internal formats, not a frozen commercial SDK. Future format revisions require explicit schema migrations before public compatibility is promised.

## Still incomplete

Full material/texture binding, OBJ `mtllib/usemtl` submesh integration, glTF, persistent import jobs, automatic reimport, and production asset-cache invalidation remain planned.
