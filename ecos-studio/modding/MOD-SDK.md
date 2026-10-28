# ECOS Studio Mod SDK — Bootstrap v1

## Goal

Let game developers and community modders package content with stable identity and explicit engine compatibility. The same tooling should eventually support ECOS and third-party games built with ECOS Studio.

## Current capability

The bootstrap creates and validates a data-only package with a mod.ecos manifest. It does not load the mod into a game, resolve dependencies, or execute scripts/native code.

## Manifest fields

- format_version=1
- id=<stable-id>: lowercase letters, digits, dot, dash, underscore; must start with a letter or digit.
- name=<display-name>
- version=<version-string>
- target_api=1
- entry=<safe relative path>: currently points to a package README or metadata entry.

Content folders:

- assets/: future supported asset imports.
- data/: future data-driven content.
- README.md: creator notes.

## Safety boundary

Do not treat community mods as trusted code. Initial support should be data-only. Before any executable plugin API is introduced, define permissions, API stability, signing/trust prompts, crash isolation, and compatibility behavior.

## Commands

    ecos-studio mod-init ./MyMod com.example.my-mod
    ecos-studio mod-validate ./MyMod
