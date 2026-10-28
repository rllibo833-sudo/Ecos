# Input Event Model

This platform-neutral input layer lets desktop adapters translate operating-system events without coupling engine code to a window library or key-code namespace.

- `InputEventQueue` is a mutex-protected multi-producer queue; `drain()` transfers the current batch in insertion order.
- `InputState::begin_frame()` clears per-frame press/release edges and wheel deltas while preserving held keys/buttons and the last pointer position.
- Repeated key-down events do not emit duplicate press edges.
- `focus_lost` releases every held key and mouse button to avoid stuck input after an application loses focus.
- Wheel deltas accumulate during a frame; adapters must normalize their units.

Text/IME, physical key mapping, gamepad discovery, raw mouse mode, and OS window lifecycle belong in platform adapters and higher-level action mapping. This core module deliberately does not pretend to open a desktop window.
