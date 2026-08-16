# FEATURE: AUTO FIGHT ON/OFF

## Purpose
Turn the client's built-in Train/Đánh quái automation ON and OFF semantically from an external test controller.

## Current Implementation
```text
button ON
 -> Command::StartAutoFight
 -> bridge SetAutoFight(true)
 -> LuaSystemManager.ExecuteFunction("TopIcon","AutoTrainClick",...)
 -> shipped wrapper resolves AutoFight_Main
 -> StartAutoFight(Train)

button OFF
 -> Command::StopAutoFight
 -> bridge SetAutoFight(false)
 -> LuaSystemManager.ExecuteFunction("TopIcon","AutoStopClick",...)
 -> StartAutoFight(None)
```

## Current Runtime Status
`RUNTIME UNTESTED`.

A successful bridge return means only that the selected invocation returned without a captured managed exception. It does **not** prove the game entered/exited Train mode.

## Current Known-Good
`UNKNOWN`.

## Related REQ / BUG / DEC
- REQ-002 AutoFight ON/OFF.
- BUG-001 execution thread boundary.
- BUG-002 ExecuteFunction runtime compatibility.
- DEC-001 semantic wrappers over visual click.

## Version Timeline

### v0.1.0
- request: isolate AutoFight ON/OFF test.
- implementation: TopIcon semantic Lua wrappers through experimental ExecuteFunction adapter.
- build: pending CI initially.
- runtime: untested.
- result: pending user live test.

## Failed / Reverted Approaches
- Visual `Đánh quái` tab click as start command: rejected because it is configuration UI, not the semantic Train start.
- Cached/stale `UIButton.HandleClickEvent`: rejected for this feature.

## Important APIs / Timing / Constants
```text
C_AutoModel.None = 0
C_AutoModel.Train = 1
TopIcon.AutoTrainClick
TopIcon.AutoStopClick
LuaSystemManager.ExecuteFunction
```

No fixed delay is considered success proof.

## Do-Not-Break Rules
- Do not replace semantic start/stop with mouse coordinates.
- Do not treat build/CI as runtime success.
- Do not add target/skill/combat reconstruction to this test; the built-in engine already owns that behavior.
- Do not modify the reference DATA repo for this test implementation.
- On failure, diagnose the external invocation bridge narrowly.

## Open Questions
- Exact live `ExecuteFunction` overload.
- Singleton/static method behavior.
- Thread-context safety.
- Best read-only runtime proof for active AutoFight mode.

## Next Diagnostic Step
Run v0.1.0 on the frozen client and record:
```text
ON button result
OFF button result
bridge log
in-game visible behavior
crash/disconnect/no-op
```
