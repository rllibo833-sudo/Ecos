# Wavefront MTL Material Import

The material importer reads a safe, dependency-free subset of Wavefront MTL:
- newmtl material declarations with duplicate-name detection
- Kd diffuse RGB
- d opacity and Tr transparency (converted to opacity)
- Ns shininess and illum illumination-model integer
- finite numeric validation, non-negative diffuse/shininess values, and opacity bounds

Texture map directives and other MTL properties are currently ignored because image decoding, texture assets, and material binding are not implemented. OBJ mtllib/usemtl linkage is also not yet integrated into mesh submeshes; this module parses material-library text independently.
