# Project Manifest Format v1

The bootstrap uses a UTF-8 line-based manifest named project.ecos. Blank lines and lines beginning with # are ignored. Each other line must be key=value; duplicate keys are rejected.

Required fields:
- format_version=1
- name=<display name>
- project_type=game
- world_mode=streamed or world_mode=single_scene
- entry_scene=<relative path to a .ecscene document>

Paths must be relative and cannot escape the project root. The validator checks that the entry file exists and, for .ecscene files, validates the scene document's schema and hierarchy.

Example:
```
format_version=1
name=My Game
project_type=game
world_mode=streamed
entry_scene=scenes/main.ecscene
```

This is an early format, not a promise that the final commercial SDK format is frozen. Future schema migrations must be explicit and documented.
