# Engine frame loop
EngineLoop composes the backend-neutral InputState and fixed-step FrameClock. Each frame:
1. Advances elapsed time with frame-delta and catch-up bounds.
2. Captures one input snapshot shared by all fixed updates and the render callback.
3. Calls fixed_update zero or more times with a stable fixed delta.
4. Calls render at most once with interpolation alpha.
5. Clears frame-transient mouse/scroll deltas after callbacks consume them.
The loop does not create a native window, poll OS events, own a GPU context, or launch its own thread. A future desktop host should pump platform events into input(), measure elapsed wall time, and call frame(). Callback exceptions currently propagate to the host; error reporting and lifecycle integration remain future work.