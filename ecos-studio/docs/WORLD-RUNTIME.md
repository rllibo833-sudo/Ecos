# World Runtime Foundation v0

The initial world runtime models world cells as project-relative content records with stable IDs, region IDs, and integer 3D cell coordinates. A cell lifecycle is:

`unloaded → loading → loaded`

Failures move `loading → failed`; failed cells can be retried, and loaded/failed cells can be returned to unloaded. Loading requests enforce a configurable resident/loading-cell budget. A radius query returns matching cell IDs in deterministic lexical order.

This is the data and lifecycle foundation for streaming, not asynchronous I/O yet. The caller is responsible for loading cell contents and reporting completion. Future work includes a versioned serialized manifest, region hierarchy, worker queue, cancellation, prioritization, eviction policy, LOD, and streaming telemetry.

The model uses integer cell coordinates to avoid accumulating precision loss across the country-scale ECOS world. Rendering will later convert local coordinates relative to a floating origin.
