# Backend-neutral input state
The input layer stores a deterministic per-frame snapshot and has no platform dependencies. A future SDL/GLFW/native backend can translate OS events into key, mouse-button, cursor, scroll, and quit events without leaking platform types into the engine core.
- Key/button press, repeat and release state.
- Finite cursor coordinates and accumulated cursor delta/scroll for each frame.
- Explicit end-of-frame transient reset and full reset.
- Bounds checks for key and mouse-button identifiers.
This is not an OS input adapter and does not create a window. The engine must call end_frame after consuming a snapshot.