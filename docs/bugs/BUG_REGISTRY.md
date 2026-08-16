# BUG REGISTRY

## BUG-001 — AutoFight mutation is not yet dispatched through canonical MainThread Action queue

Status: OPEN
Severity: High for production; acceptable only as explicit experiment
First Observed: v0.1.0 design review
Last Tested: v0.1.2 BUILD/CI only; live v0.1.2 mutation not yet tested
Last Known-Good: UNKNOWN
First Confirmed-Bad: UNKNOWN
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence:
- DATA KB verifies `FGStudio.Engine.Utilities.MainThread.Execute(System.Action)` enqueues and Unity `Update()` drains/invokes actions.
- DATA/current-tool research verifies the WH_GETMESSAGE window-thread callback can be proven as the Unity managed main thread using `UnitySynchronizationContext` and managed thread IDs.
- v0.1.2 implements that Unity-main-thread proof before direct semantic AutoFight mutation.
Unknowns:
- whether executing `MonoBehaviourExecutor.ExecuteUiObject` directly inside the WH_GETMESSAGE callback is re-entrancy/lifecycle safe for repeated production use;
- whether v0.1.2 produces the intended live Train/None state without disconnect/crash/no-op.
Root Cause: architecture gap: correct thread identity is not the same as the preferred normal-Update action boundary.
Attempts:
- v0.1.0/v0.1.1: mutation was never reached because the ExecuteFunction adapter failed first.
- v0.1.2: prove Unity managed main-thread identity, then perform one narrow direct service action with frozen-donor guards.
Current Workaround: v0.1.2 is test-only; one action at a time; fail closed on map transition/client mismatch/thread proof failure.
Fixed In: UNKNOWN
Runtime Verified In: UNKNOWN
Next Diagnostic Step: live-test v0.1.2 ON once and OFF once. If semantic action is correct but unstable/re-entrant, move the same semantic service action behind a legitimate `System.Action -> MainThread.Execute` boundary.
Do-Not-Do: do not substitute mouse macros, stale UIButton pointers or `CreateRemoteThread` gameplay workers.

## BUG-002 — v0.1.0/v0.1.1 ExecuteFunction adapter assumptions did not match the live client

Status: CHARACTERIZED / SUPERSEDED FOR AUTOFIGHT IN v0.1.2
Severity: Medium
First Observed: v0.1.0 implementation
First Runtime Confirmed: 2026-08-16
Last Tested: v0.1.1
Last Known-Good: NONE
First Confirmed-Bad: v0.1.0
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence:
- v0.1.0: `Bridge probe PASS; chưa gửi AutoFight action`.
- v0.1.0 ON: `ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`.
- v0.1.1 live metadata dump:
  - `static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue`
  - `static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue`
Root Cause:
- v0.1.0 assumed `(String,String)` / `(String,String,Object[])` shapes and used argc-oriented lookup.
- v0.1.1 corrected method enumeration and proved that neither assumed shape exists in the live client.
- The actual string overload is `(System.String,System.Object[])`, but using it for `TopIcon:AutoTrainClick()` would still leave the Lua `self`/UI-context problem.
Attempts:
- v0.1.0: fail before invoke.
- v0.1.1: exact overload enumeration + metadata dump; correctly failed closed and produced exact signatures.
Resolution:
- Preserve the exact overload information as verified metadata.
- AutoFight v0.1.2 bypasses `LuaSystemManager.ExecuteFunction` and targets the lower-level persistent service `AutoFight_Main.StartAutoFight(1/0)` instead.
Fixed In: not fixed as a generic ExecuteFunction adapter; superseded for AutoFight by v0.1.2
Runtime Verified In: exact overload discovery verified in v0.1.1; v0.1.2 AutoFight effect still UNKNOWN
Next Diagnostic Step: no further ExecuteFunction reverse work is required for the current AutoFight task unless the direct service path later proves unusable.
Do-Not-Do: do not broad reverse the client and do not guess Lua self/argument conventions.

## BUG-003 — Frozen MonoBehaviourExecutor donor RVAs are client-build specific

Status: GUARDED RISK
Severity: Medium
First Observed: v0.1.2 implementation
Last Tested: v0.1.2 BUILD/CI PASS
Last Known-Good: UNKNOWN runtime
Related Feature / REQ: AUTO_FIGHT / REQ-002
Known Evidence: prior v0.9.0 donor source for this frozen client used `MonoBehaviourExecutorGetInstance` RVA `0x523CE0`, `MonoBehaviourExecutorExecuteUiObject` RVA `0x521B20`, and IL2CPP Object[] data offset `0x20`.
Risk: these are fixed-snapshot implementation locators, not universal API contracts.
Mitigation in v0.1.2:
- require PE TimeDateStamp `0x6A410C14`;
- require SizeOfImage `0x03DCB000`;
- require exact byte signatures at both donor RVAs before calling them;
- fail closed on mismatch.
Fixed In: not applicable; deliberate guarded donor dependency for this frozen-client experiment
Runtime Verified In: UNKNOWN
Next Diagnostic Step: if v0.1.2 reports client/donor mismatch, inspect only the exact executor metadata/native locator for that client build rather than removing the guard.
