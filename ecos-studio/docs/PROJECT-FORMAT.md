# Project Manifest Format v1

The bootstrap uses a UTF-8 line-based manifest named project.ecos. Blank lines and lines beginning with # are ignored. Each other line must be key=value; duplicate keys are rejected.

Required fields:

- format_version=1: schema version.
- name=<display name>: project display name.
- project_type=game: reserved for game projects in bootstrap v1.
- world_mode=streamed or world_mode=single_scene.
- entry_scene=<relative path>: path to the initial scene metadata.

Paths must be relative and cannot contain a .. component. The bootstrap checks that the entry file exists. It does not parse or render scene contents yet.

Example:

    format_version=1
    name=My Game
    project_type=game
    world_mode=streamed
    entry_scene=scenes/main.ecos-scene

This is a bootstrap format, not a claim that the final editor's format is frozen. Future schema migrations must be explicit and documented.
