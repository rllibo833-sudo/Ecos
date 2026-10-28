# Fixed-step frame clock
The backend-independent FrameClock converts elapsed wall time into a bounded frame delta and fixed simulation steps.
- Defaults to a 1/60 second simulation step.
- Clamps long frame deltas and limits the number of fixed updates per frame.
- Reports dropped time when limits are reached instead of allowing an unbounded catch-up spiral.
- Returns interpolation alpha for rendering between simulation states.
- Ignores negative/non-finite elapsed values and supports reset.
The caller owns the wall-clock source and must call advance once per frame, then execute the returned number of fixed simulation steps using fixed_delta_seconds. This is timing infrastructure, not yet a full application loop or scheduler.