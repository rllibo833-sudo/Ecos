# World Manifest Format v1

World manifests use a UTF-8 line-oriented format named .ecworld. Blank lines and lines beginning with # are ignored. The validator rejects unknown keys, duplicate required fields, unsupported versions, duplicate cell IDs, duplicate integer cell coordinates, malformed coordinates, and paths that escape the project root.

Required fields:
- format_version=1
- world_id=<stable-world-id>
- One or more cell records: cell=id|region_id|x,y,z|relative-content-path

Example:
```
format_version=1
world_id=ecos-sample-world
cell=arunika-origin|arunika|0,0,0|world/arunika/origin.ecell
cell=arunika-east|arunika|1,0,0|world/arunika/east.ecell
```

Run the headless validator:
```
ecos-studio world-validate path/to/world.ecworld
```

The manifest is metadata only; the validator does not claim referenced cell payloads have been generated or loaded. Future format revisions must include explicit migrations before becoming a stable external SDK contract.
