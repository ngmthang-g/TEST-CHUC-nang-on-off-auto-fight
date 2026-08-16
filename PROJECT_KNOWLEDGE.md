# PROJECT KNOWLEDGE

## Project Identity
- Name: Thần Long AutoFight Test
- Repository: `ngmthang-g/TEST-CHUC-nang-on-off-auto-fight`
- Primary branch: `main`
- Current version: `v0.1.2`
- Last known-good AutoFight release: `UNKNOWN`
- Baseline source: user-supplied `ThanLongCleanRoute_v1.0.0_FIXED_GitHubActions`
- Reference knowledge repo: `ngmthang-g/clinent-game-than-long-DATA-2222` (read-only for this project)
- Platform: Windows x64, Unity/IL2CPP client, Win32 controller + injected WH_GETMESSAGE bridge
- Build: Zig C++ / GitHub Actions Windows runner

## Project Goal
Prove a minimal reliable way to turn built-in Thần Long AutoFight Train mode ON and OFF semantically, without screen/mouse macros and without treating the visible `Đánh quái` settings tab as the start action.

## Current State

### Runtime-confirmed facts
- v0.1.0: `Bridge probe PASS; chưa gửi AutoFight action`.
- v0.1.0 ON: `ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`; no Lua action was invoked.
- v0.1.1 Probe: PASS.
- v0.1.1 ON produced the exact live `LuaSystemManager.ExecuteFunction` overload set:
  - `static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue`
  - `static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue`
- Therefore the old assumptions `(String,String)` / `(String,String,Object[])` are false for this live client.
- v0.1.1 still did not issue the AutoFight mutation because it failed closed after dumping metadata.

### Important donor/runtime-history correction
Prior AutoTrain source/runtime history from the same frozen client shows that directly executing `TopIcon.AutoTrainClick` through the old MonoBehaviourExecutor path could dispatch the callback without changing `EnableAutoF1`. The `TopIcon` wrapper also contains `self:ShowAutoStatus(...)`, so it is more context-dependent than the underlying service.

The lower-level semantic target is therefore:
- ON: `AutoFight_Main:StartAutoFight(C_AutoModel.Train)` where `Train = 1`.
- OFF: `AutoFight_Main:StartAutoFight(C_AutoModel.None)` where `None = 0`.

This is also the preferred semantic service documented in the DATA knowledge base.

### v0.1.2 implementation
v0.1.2 no longer uses `LuaSystemManager.ExecuteFunction` as the mutation path. It performs:

```text
WH_GETMESSAGE bridge callback
 -> map-transition safety guards
 -> prove current managed SynchronizationContext == UnitySynchronizationContext
 -> prove current ManagedThreadId == Unity main-thread ID
 -> validate exact frozen GameAssembly identity
 -> validate byte signatures for frozen MonoBehaviourExecutor donor RVAs
 -> LuaSystemAPI_GUI.FindUI/MainFindUI("AutoFight_Main")
 -> box System.Int32 mode (1 or 0)
 -> create/root System.Object[1]
 -> MonoBehaviourExecutor.ExecuteUiObject(service,"StartAutoFight",[mode])
 -> read immediate EnableAutoF1 when available
 -> return diagnostic; user must still confirm real in-game behavior
```

Protocol: `0x00020200`, preventing v0.1.1 EXE/DLL from silently mixing with v0.1.2.

### v0.1.2 guards
- No mouse input or screen coordinates.
- No `CreateRemoteThread` gameplay worker.
- No guessed UIButton pointer.
- Requires exact frozen PE timestamp `0x6A410C14` and SizeOfImage `0x03DCB000` before frozen donor RVAs are callable.
- Requires donor byte signatures at:
  - MonoBehaviourExecutor get instance RVA `0x523CE0`.
  - ExecuteUiObject RVA `0x521B20`.
- Requires live `AutoFight_Main` service object.
- Requires Unity managed main-thread proof before mutation.
- Uses strong IL2CPP GC handles for the boxed mode, Object[] and function-name string during the call.

### v0.1.2 Build / CI
- First v0.1.2 CI run `31938163761` on head `e3a1241262aec66abe3195f2bc699eb4c1e1da32`:
  - scope audit PASS;
  - inherited route FSM `8/8 PASS`;
  - compile then FAILED only because the retained v0.1.1 helper `AppendExecuteFunctionSignatures` became unused under `-Werror`.
- Fix commit `1aed2b89f0d3aa1d8ea30774ac45078f872fd940` preserves the legacy diagnostic symbol as intentionally referenced.
- Corrected CI run `31938259381`:
  - Build and audit PASS;
  - Upload test binaries PASS.
- Artifact: `ThanLong-AutoFight-Test-v0.1.2`, ID `9261301912`, size `180271` bytes.
- Artifact digest: `sha256:03d0bc330cde908a574ec0147876dea8310126313078ad44af73ea095e2ff7ca`.
- v0.1.2 AutoFight ON/OFF runtime: `UNTESTED` until user runs it on the live client.

## Architecture

### Current experimental v0.1.2 path
```text
Controller
 -> per-PID shared memory
 -> WH_GETMESSAGE hook
 -> proven Unity managed main-thread callback
 -> safety + frozen-donor validation
 -> semantic AutoFight_Main service
 -> MonoBehaviourExecutor.ExecuteUiObject(StartAutoFight, mode)
 -> immediate state read + external runtime observation
```

### Canonical production target
```text
Resolver
 -> read-only scanner/snapshot
 -> observer/state machine
 -> safety guard
 -> max one action
 -> legitimate System.Action
 -> FGStudio.Engine.Utilities.MainThread.Execute
 -> Unity Update
 -> semantic Lua/Game/UI action
 -> real state proof
```

v0.1.2 proves/uses the Unity-main-thread identity, but it still calls the semantic action from the WH_GETMESSAGE callback rather than enqueueing a `System.Action` through `MainThread.Execute`. That remains an explicit experimental architecture gap.

## Open Bugs / Risks
- `BUG-001`: production mutation boundary still does not use `MainThread.Execute(System.Action)`; v0.1.2 reduces arbitrary-thread risk by proving Unity main-thread identity, but re-entrancy/lifecycle safety is not yet proven.
- `BUG-002`: old ExecuteFunction adapter mismatch is now fully characterized by live v0.1.1 metadata and is superseded for AutoFight by the v0.1.2 direct-service path.
- Frozen donor RVAs are accepted only after exact PE + byte-signature validation; they are not universal client constants.

## Known-Good Summary
- Bridge Probe: runtime PASS.
- Exact live ExecuteFunction metadata: runtime VERIFIED by v0.1.1.
- AutoFight ON/OFF semantic effect: no known-good version yet.
- v0.1.2: BUILD/CI PASS, runtime UNTESTED.

## Current Test Priority
1. Use only the v0.1.2 EXE + DLL pair.
2. Probe once.
3. Press ON once while alive and map is stable.
4. Preserve the exact ON log and observe whether built-in Train/Đánh quái actually starts.
5. If ON works, press OFF once and preserve the OFF log/state.
6. If the process disconnects/crashes/no-ops, diagnose the exact v0.1.2 failure boundary; do not fall back to mouse clicks.

## Hard Rules
- Do not mark runtime PASS from CI or from a native function merely returning.
- Do not broad reverse while the DATA KB or preserved donor source already contains exact facts.
- Do not revive the old `TopIcon` wrapper as the preferred action path unless new evidence requires it.
- DATA repo remains reference-only.
- Preserve every runtime result and correction in this repo.

## Knowledge Index
- Feature: `docs/features/AUTO_FIGHT.md`
- v0.1.0: `docs/history/VERSION_v0.1.0.md`
- v0.1.1: `docs/history/VERSION_v0.1.1.md`
- v0.1.2: `docs/history/VERSION_v0.1.2.md`
- Bugs: `docs/bugs/BUG_REGISTRY.md`
- Decisions: `docs/decisions/DECISIONS.md`
- Evidence: `docs/evidence/EVIDENCE_REGISTRY.md`
- Changelog: `CHANGELOG.md`
