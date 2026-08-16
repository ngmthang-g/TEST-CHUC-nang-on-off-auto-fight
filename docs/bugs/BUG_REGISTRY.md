# BUG REGISTRY

## BUG-001 — AutoFight mutation is not yet dispatched through canonical MainThread Action queue

Status: OPEN
Severity: High for production; acceptable only as explicit experiment
First Observed: v0.1.0 design review
Last Tested: v0.1.0 bridge probe only; mutable Lua action was not reached because BUG-002 failed first
Last Known-Good: UNKNOWN
First Confirmed-Bad: UNKNOWN
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence: DATA KB states mutable production actions should use valid `System.Action -> MainThread.Execute`. User runtime evidence confirms the inherited bridge callback can execute the non-mutating Probe command, but does not establish mutable Lua safety.
Unknowns: whether the inherited WH_GETMESSAGE callback context is safe for this exact Lua wrapper.
Root Cause: architecture gap, not yet a demonstrated runtime fault.
Attempts: v0.1.0 intentionally reuses inherited bridge only for narrow proof.
Current Workaround: test one action at a time and fail closed during map transition.
Fixed In: UNKNOWN
Runtime Verified In: UNKNOWN
Next Diagnostic Step: first resolve BUG-002. Only diagnose thread boundary if a matched ExecuteFunction call reaches invocation and then crashes/disconnects/no-ops.
Do-Not-Do: do not substitute mouse macros or `CreateRemoteThread` gameplay worker.

## BUG-002 — v0.1.0 ExecuteFunction overload resolver rejected the live client signature

Status: FIX CANDIDATE IN v0.1.1; RUNTIME UNVERIFIED
Severity: Medium
First Observed: v0.1.0 implementation
First Runtime Confirmed: 2026-08-16 user test
Last Tested: v0.1.0
Last Known-Good: UNKNOWN
First Confirmed-Bad: v0.1.0
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence:
- User runtime log: `Bridge probe PASS; chưa gửi AutoFight action`.
- User runtime log: `ON FAIL: ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`.
- Therefore bridge attach/callback/protocol reached the in-process adapter, but no AutoFight mutable Lua call was issued.
Root Cause: v0.1.0 used `il2cpp_class_get_method_from_name(name, argc)` and then validated only the single returned method. This is insufficient when multiple overloads share the same parameter count; it can select a different overload and reject it even if a supported overload also exists. The exact live overload set is still runtime-dependent until v0.1.1 reports/matches it.
Attempts:
- v0.1.0: supported only `(String,String)` and `(String,String,Object[])` after argc-based lookup -> runtime FAIL before invoke.
- v0.1.1: enumerate all methods with `il2cpp_class_get_methods`, match name + exact parameter types, and dump all `ExecuteFunction` signatures if neither known form matches.
Current Workaround: use v0.1.1 only; do not retry v0.1.0.
Fixed In: v0.1.1 candidate
Runtime Verified In: UNKNOWN
Next Diagnostic Step: run v0.1.1. If it matches a supported overload, observe ON/OFF behavior. If it still fails, preserve the full `ExecuteFunction overloads: ...` log, which is now the exact narrow metadata needed for the next adapter correction.
Do-Not-Do: do not broad reverse the entire client and do not guess arbitrary overload payloads.
