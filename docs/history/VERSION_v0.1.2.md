# VERSION v0.1.2 — Direct AutoFight_Main service experiment

Date: 2026-08-16
Status: BUILD PASS / CI PASS / RUNTIME UNTESTED

## Triggering user runtime evidence
v0.1.1 live log:

```text
Bridge probe PASS; chưa gửi AutoFight action
ON FAIL: ExecuteFunction overloads: static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue | static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue
```

This established the exact `LuaSystemManager.ExecuteFunction` overloads and disproved the v0.1.0 assumed signatures.

## Analysis decision
Do not immediately call the newly-discovered `(System.String,System.Object[])` overload with `TopIcon.AutoTrainClick`.

Reason:
- `TopIcon:AutoTrainClick()` is a colon method and uses `self:ShowAutoStatus(...)`.
- preserved old runtime history shows direct `TopIcon.AutoTrainClick` dispatch could return without changing `EnableAutoF1`.
- the underlying persistent service is exactly `AutoFight_Main:StartAutoFight(mode)`.

Therefore v0.1.2 targets the service directly:
- ON = `StartAutoFight(1)` / `C_AutoModel.Train`.
- OFF = `StartAutoFight(0)` / `C_AutoModel.None`.

## Implementation
Action sequence:

```text
controller command
 -> WH_GETMESSAGE callback on selected game window thread
 -> IsMapReady / WaitingChangeMap guards
 -> SynchronizationContext.Current must be UnitySynchronizationContext
 -> Thread.CurrentThread.ManagedThreadId must equal Unity main-thread ID
 -> exact frozen client PE identity check
 -> exact byte signatures for MonoBehaviourExecutor donor RVAs
 -> LuaSystemAPI_GUI.FindUI/MainFindUI("AutoFight_Main")
 -> box System.Int32 mode
 -> allocate System.Object[1]
 -> root boxed mode / array / function string with IL2CPP GC handles
 -> MonoBehaviourExecutor.ExecuteUiObject(AutoFight_Main,"StartAutoFight",args)
 -> attempt immediate EnableAutoF1 read
 -> return human diagnostic
```

## Frozen donor facts used
- GameAssembly TimeDateStamp: `0x6A410C14`.
- GameAssembly SizeOfImage: `0x03DCB000`.
- `MonoBehaviourExecutorGetInstance` RVA: `0x523CE0`.
- `MonoBehaviourExecutorExecuteUiObject` RVA: `0x521B20`.
- IL2CPP x64 Object[] first-element data offset from donor: `0x20`.
- Both donor RVAs are byte-signature checked before use.

## Protocol
- `kProtocolVersion = 0x00020200`.
- This prevents v0.1.1 and v0.1.2 EXE/DLL pair mixing.

## Build history
### Run 31938163761 — FAILED
Head: `e3a1241262aec66abe3195f2bc699eb4c1e1da32`

Passed before failure:
- new semantic/main-thread/donor scope audit;
- inherited route FSM 8/8.

Failure:
```text
src/bridge_runtime.inl:202:6: error: unused function 'AppendExecuteFunctionSignatures' [-Werror,-Wunused-function]
```

Interpretation: legacy v0.1.1 diagnostic helper became unused after the action path moved away from ExecuteFunction.

### Fix
Commit: `1aed2b89f0d3aa1d8ea30774ac45078f872fd940`

The legacy exact-overload diagnostic helper remains intentionally referenced as preserved evidence/diagnostic code.

### Run 31938259381 — PASS
- Build and audit PASS.
- Upload test binaries PASS.
- Artifact: `ThanLong-AutoFight-Test-v0.1.2`.
- Artifact ID: `9261301912`.
- Size: `180271` bytes.
- SHA256: `03d0bc330cde908a574ec0147876dea8310126313078ad44af73ea095e2ff7ca`.

## Runtime status
- Probe behavior already known-good from prior versions.
- v0.1.2 ON: UNTESTED.
- v0.1.2 OFF: UNTESTED.
- Known-good AutoFight effect: UNKNOWN.

## Important architecture caveat
v0.1.2 proves the callback is on the Unity managed main thread before mutation, but it still invokes `ExecuteUiObject` from the WH_GETMESSAGE callback rather than enqueueing a legitimate `System.Action` through `FGStudio.Engine.Utilities.MainThread.Execute`.

Therefore v0.1.2 is an experimental semantic proof, not the final production action boundary.

## Next runtime test
1. Use the v0.1.2 EXE and DLL from the same artifact.
2. Probe.
3. Press ON once while character is alive and map stable.
4. Record full ON log and visible game behavior.
5. If ON truly starts Train, press OFF once and record full OFF log/state.
6. Any crash/disconnect/no-op must be recorded before changing the mechanism.
