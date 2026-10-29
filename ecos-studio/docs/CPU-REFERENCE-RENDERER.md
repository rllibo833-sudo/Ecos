# CPU Reference Renderer v0

The reference renderer turns indexed triangle meshes into binary PPM (P6) images without external libraries or a GPU. It includes bounded image sizing, mesh/index/finite-coordinate validation, orthographic framing from mesh bounds, triangle rasterization, a depth buffer, and deterministic directional diffuse shading.

Example:

```sh
ecos-studio mesh-render examples/hello-world/assets/triangle.obj triangle.ppm 640 480
```

The output is intended for automated verification, deterministic reference output, and basic asset previews. This CPU tool is separate from the native desktop viewport: the viewport now has an experimental GLFW/OpenGL GPU path, but that path is not a production renderer.

The CPU renderer does not provide a window, GPU acceleration, perspective camera, textures, full material binding, anti-aliasing, or a production clipping pipeline. Keep this reference path useful for tests even as the GPU renderer evolves.
