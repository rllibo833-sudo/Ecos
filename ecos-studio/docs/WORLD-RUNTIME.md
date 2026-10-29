# World Runtime and Viewport Streaming

## Runtime foundation

The world runtime models cells as project-relative content records with stable IDs, region IDs, and integer 3D cell coordinates. Lifecycle transitions include unloaded, loading, loaded and failed states; retries, unloading, resident/loading budgets and deterministic radius queries are supported by the foundational model.

## Native viewport integration — experimental

The native editor updates the world-content runtime from the viewport target position. World coordinates in meters are converted to 1 km cell coordinates before updating the requested resident neighborhood. Loaded cell scene snapshots are rendered with cell offsets applied to local transforms. A distance-based LOD path substitutes a box proxy for far meshes, with local dimensions derived from the source mesh's axis-aligned bounds.

This is an initial integration, not production streaming. Asynchronous disk I/O, worker scheduling, cancellation, prioritization, robust eviction, floating origin, streaming telemetry, and distant simulation tiers remain open work. Do not interpret manifest validation as proof that all referenced cell payloads exist or are loaded.

## Large-world precision

Integer cell coordinates limit precision loss across the country-scale ECOS world. A future production renderer must define floating-origin behavior and explicit memory, entity, and streaming budgets.
