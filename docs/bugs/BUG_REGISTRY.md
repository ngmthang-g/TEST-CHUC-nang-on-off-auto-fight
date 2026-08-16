# BUG REGISTRY

## BUG-001 — AutoFight mutation is not yet dispatched through canonical MainThread Action queue

Status: OPEN
Severity: High for production; acceptable only as explicit experiment
First Observed: v0.1.0 design review
Last Tested: not runtime-tested
Last Known-Good: UNKNOWN
First Confirmed-Bad: UNKNOWN
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence: DATA KB states mutable production actions should use valid `System.Action -> MainThread.Execute`.
Unknowns: whether the inherited WH_GETMESSAGE callback context happens to be safe for this exact Lua wrapper.
Root Cause: architecture gap, not yet a demonstrated runtime fault.
Attempts: v0.1.0 intentionally reuses inherited bridge only for narrow proof.
Current Workaround: test one action at a time and fail closed during map transition.
Fixed In: UNKNOWN
Runtime Verified In: UNKNOWN
Next Diagnostic Step: run v0.1.0; if instability/no-op points to thread boundary, implement MainThread Action bridge.
Do-Not-Do: do not substitute mouse macros or `CreateRemoteThread` gameplay worker.

## BUG-002 — Exact live LuaSystemManager.ExecuteFunction adapter compatibility unverified

Status: OPEN
Severity: Medium
First Observed: v0.1.0 implementation
Last Tested: not runtime-tested
Last Known-Good: UNKNOWN
First Confirmed-Bad: UNKNOWN
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence: method existence is established in DATA KB; exact v0.1.0 external overload/instance invocation has no live proof yet.
Unknowns: overload parameter list, static/instance ownership, script lifetime at action time.
Root Cause: UNKNOWN until runtime evidence.
Attempts: support two narrow forms: `(String,String)` and `(String,String,Object[])` plus static/singleton instance handling.
Current Workaround: fail with diagnostic instead of guessing more signatures.
Fixed In: UNKNOWN
Runtime Verified In: UNKNOWN
Next Diagnostic Step: use returned diagnostic; inspect exact metadata only if mismatch is reported.
Do-Not-Do: do not broad reverse the entire client.
