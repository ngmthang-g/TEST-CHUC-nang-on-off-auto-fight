# PROJECT KNOWLEDGE

## Project Identity
- Name: Thần Long AutoFight Test
- Repository: `ngmthang-g/TEST-CHUC-nang-on-off-auto-fight`
- Primary branch: `main`
- Current version: `v0.1.0`
- Last known-good AutoFight release: `UNKNOWN`
- Baseline source: user-supplied `ThanLongCleanRoute_v1.0.0_FIXED_GitHubActions`
- Reference knowledge repo: `ngmthang-g/clinent-game-than-long-DATA-2222` (read-only for this project)
- Platform: Windows x64, Unity/IL2CPP client, Win32 controller + injected hook bridge
- Build: Zig C++ / GitHub Actions Windows runner

## Project Goal
Prove a minimal reliable way to turn built-in Thần Long AutoFight Train mode ON and OFF semantically, without mouse macros or clicking the visible settings tab.

## Current State

### Runtime-confirmed working
- `UNKNOWN` for this new AutoFight test repo. No live-client runtime result has been reported yet.

### Build / CI confirmed
- `BUILD PASS` / `CI PASS` on GitHub Actions run `31935017087` for commit `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0`.
- AutoFight scope audit PASS.
- Inherited route FSM regression test `8/8 PASS`.
- `ThanLongAutoFightTestBridge.dll` compiled and passed PE DLL validation.
- `ThanLongAutoFightTest_v0.1.0.exe` compiled successfully.
- Artifact `ThanLong-AutoFight-Test-v0.1.0` was uploaded successfully as artifact ID `9260424827`.

### Built but runtime-untested
- Protocol commands `StartAutoFight` and `StopAutoFight`.
- Controller buttons for ON/OFF.
- IL2CPP resolver path for `FGStudio.LuaSystem.LuaSystemManager`.
- Experimental invocation of `ExecuteFunction` for `TopIcon.AutoTrainClick` / `TopIcon.AutoStopClick`.

### Partial / unstable
- External action execution context is not yet the canonical production `MainThread.Execute(System.Action)` path.
- `LuaSystemManager.ExecuteFunction` overload/instance behavior is source-informed but not yet runtime-proven by this tool.

### Known regressions
- None established yet.

### Open bugs / risks
- `BUG-001`: AutoFight mutable Lua call is executed through inherited hook callback context rather than a proven MainThread Action dispatcher.
- `BUG-002`: exact live compatibility of the chosen `ExecuteFunction` overload is unverified.

### Current priorities
1. Run ON test on the live client.
2. Run OFF test on the live client.
3. Record the exact tool log plus visible in-game behavior.
4. Update runtime evidence immediately after the test.
5. If no-op/crash/disconnect occurs, inspect only the narrow bridge/signature/thread boundary before changing architecture.

## Architecture

### Current test architecture
```text
Win32 controller
 -> per-PID shared memory
 -> WH_GETMESSAGE hook bridge
 -> IL2CPP metadata/API resolution
 -> LuaSystemManager.ExecuteFunction
 -> TopIcon.AutoTrainClick / AutoStopClick
 -> AutoFight_Main.StartAutoFight(Train/None)
```

### Canonical production target from DATA knowledge
```text
Resolver
 -> read-only snapshot
 -> state machine / safety guard
 -> max one mutable action
 -> valid System.Action
 -> MainThread.Execute
 -> semantic Lua/Game/UI action
 -> state proof
```

## Major Components
- `src/controller.cpp` + `src/controller_support.inl`: PID discovery, bridge attach, ON/OFF UI, runtime log.
- `src/bridge.cpp` + `src/bridge_runtime.inl` + `src/bridge_lua.inl`: in-process IL2CPP resolution and action adapter.
- `src/protocol.h`: shared-memory protocol.
- `src/route_logic.*`: inherited baseline regression test/support; not exposed by the AutoFight test UI.
- `.github/workflows/build.yml`: reproducible Windows build and binary artifact.

## Hard Architectural Rules
- Do not implement AutoFight as screen/mouse clicking.
- Do not treat the visible `Đánh quái` tab as the start action.
- Do not broad reverse-engineer the client while exact facts are present in the DATA knowledge base.
- Do not call `UIButton.HandleClickEvent` using guessed/null/stale instances.
- Do not mark runtime PASS from build/CI.
- If thread context is the failure, fix the execution bridge rather than replacing the semantic AutoFight contract.
- DATA repo is reference-only for this task.

## Known-Good Summary
- AutoFight test: `UNKNOWN` until live runtime evidence exists.

## Failed / Unsafe Mechanisms
- Visual mouse macro for AutoFight: intentionally rejected.
- Clicking the visible `Đánh quái` tab as start: disproven by reference KB semantics.
- Production arbitrary-thread direct mutable Unity/Lua action: not accepted as canonical architecture.

## Important Technical Facts
- `C_AutoModel.None = 0`.
- `C_AutoModel.Train = 1`.
- `TopIcon.AutoTrainClick()` is the shipped zero-argument Train start wrapper.
- `TopIcon.AutoStopClick()` is the shipped zero-argument stop wrapper.
- `LuaSystemManager` exposes `ExecuteFunction` in reference metadata/source analysis.
- The initial Windows CI failure was an audit encoding problem, not a C++ AutoFight compile failure: PowerShell read UTF-8 Vietnamese UI text without explicit UTF-8 decoding.
- Correction commit `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0` changed audit reads to `Get-Content -Encoding UTF8`; the subsequent CI passed.

## Important IDs / APIs / Data Sources
- `FGStudio.LuaSystem.LuaSystemManager`
- `ExecuteFunction`
- `TopIcon:AutoTrainClick`
- `TopIcon:AutoStopClick`
- `GUI.FindUI("AutoFight_Main")`
- `AutoFight_Main:StartAutoFight(...)`

## Persistence / Configuration
No AutoFight settings are modified by this test. The built-in game AutoFight configuration remains owned by the client.

## Build Environment
- GitHub Actions: `windows-latest`
- Zig: 0.14.1
- local build entry: `build.cmd`
- Passing CI run: `31935017087`
- Passing source/build commit: `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0`
- Build artifact: `ThanLong-AutoFight-Test-v0.1.0`, artifact ID `9260424827`
- Artifact digest: `sha256:f716e2a33eba0a9eea7cc889b9ab19d3f4ea4c1c10db19856a732f1167223598`

## Open Questions
- Exact runtime overload selected for `ExecuteFunction`.
- Whether the manager method is static or requires a live singleton on this frozen build.
- Whether inherited hook callback execution is acceptable for this Lua call.
- Runtime-visible proof source beyond user observation for a future version.

## Knowledge Index
- Feature: `docs/features/AUTO_FIGHT.md`
- Version: `docs/history/VERSION_v0.1.0.md`
- Bugs: `docs/bugs/BUG_REGISTRY.md`
- Decisions: `docs/decisions/DECISIONS.md`
- Evidence: `docs/evidence/EVIDENCE_REGISTRY.md`
- Changelog: `CHANGELOG.md`
