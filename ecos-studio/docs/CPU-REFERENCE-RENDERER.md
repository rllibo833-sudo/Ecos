# CPU Reference Renderer v0

The reference renderer turns indexed triangle meshes into binary PPM (P6) images without external libraries or a GPU. It includes a bounded image size, mesh/index/finite-coordinate validation, orthographic framing from mesh bounds, triangle rasterization, a depth buffer, and deterministic directional diffuse shading.

Example:

```sh
ecos-studio mesh-render examples/hello-world/assets/triangle.obj triangle.ppm 640 480
```

The output can be opened by common image viewers or converted by standard image tools. The renderer is intended for automated verification, asset previews, and as a deterministic reference implementation. It is not the production desktop viewport: no window, GPU acceleration, perspective camera, textures, material system, anti-aliasing, or clipping pipeline is provided. The planned desktop renderer remains a separate GPU-backed subsystem.
