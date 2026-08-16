# PROJECT KNOWLEDGE

## Project Identity
- Name: Thần Long AutoFight Test
- Repository: `ngmthang-g/TEST-CHUC-nang-on-off-auto-fight`
- Primary branch: `main`
- Current version: `v0.1.1`
- Last known-good AutoFight release: `UNKNOWN`
- Baseline source: user-supplied `ThanLongCleanRoute_v1.0.0_FIXED_GitHubActions`
- Reference knowledge repo: `ngmthang-g/clinent-game-than-long-DATA-2222` (read-only for this project)
- Platform: Windows x64, Unity/IL2CPP client, Win32 controller + injected hook bridge
- Build: Zig C++ / GitHub Actions Windows runner

## Project Goal
Prove a minimal reliable way to turn built-in Thần Long AutoFight Train mode ON and OFF semantically, without mouse macros or clicking the visible settings tab.

## Current State

### Runtime-confirmed facts
- v0.1.0 live user test: `Bridge probe PASS; chưa gửi AutoFight action`.
- v0.1.0 live user test: `ON FAIL: ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`.
- Therefore controller/attach/shared-memory/hook callback/protocol reached the in-process bridge successfully for Probe.
- AutoFight ON failed before `TopIcon.AutoTrainClick` invocation; semantic AutoFight behavior itself was not tested in that attempt.

### v0.1.1 correction
- Replaced argc-only `ExecuteFunction` lookup with full overload enumeration via `il2cpp_class_get_methods` + `il2cpp_method_get_name`.
- Exact matching checks name + argc + every IL2CPP parameter type.
- The tool still only invokes known intended shapes:
  - `(System.String,System.String)`
  - `(System.String,System.String,System.Object[])`
- If neither exists, v0.1.1 returns the exact live `ExecuteFunction overloads: ...` list and does not guess arguments.
- Protocol is `0x00020100` so v0.1.0 and v0.1.1 EXE/DLL files cannot silently mix.

### v0.1.1 build / CI confirmed
- `BUILD PASS` / `CI PASS` on GitHub Actions run `31937194750` for head `3a90b6ca7e69e08d610491ae70231ceaf92ec1e6`.
- Build/audit step PASS.
- Artifact upload PASS.
- Artifact: `ThanLong-AutoFight-Test-v0.1.1`, ID `9261010029`, size `117381` bytes.
- Artifact digest: `sha256:3faa3b25be998c3aa65830228affc89a8efc095a84f3bacf158b93d7715dab12`.
- Runtime AutoFight ON/OFF remains `UNTESTED` for v0.1.1.

### Open bugs / risks
- `BUG-001`: mutable Lua action still uses inherited hook callback context instead of canonical `MainThread.Execute(System.Action)`; not yet reached/tested by v0.1.0 due BUG-002 failing first.
- `BUG-002`: v0.1.0 adapter mismatch confirmed; v0.1.1 is the exact-overload runtime fix candidate.

### Current priorities
1. Run v0.1.1 Probe, then ON once.
2. If ON reports `ExecuteFunction exact 2-param matched` or `exact 3-param matched`, observe actual game behavior and test OFF.
3. If ON still fails, preserve the full `ExecuteFunction overloads: ...` line.
4. Only investigate MainThread/thread safety after a compatible method is actually invoked.

## Architecture

### Current test architecture
```text
Win32 controller
 -> per-PID shared memory
 -> WH_GETMESSAGE hook bridge
 -> IL2CPP metadata/API resolution
 -> exact ExecuteFunction overload enumeration
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

## Hard Architectural Rules
- Do not implement AutoFight as screen/mouse clicking.
- Do not treat the visible `Đánh quái` tab as the start action.
- Do not broad reverse-engineer the client while exact facts are present in the DATA knowledge base.
- Do not call `UIButton.HandleClickEvent` using guessed/null/stale instances.
- Do not mark runtime PASS from build/CI or from a method returning without exception.
- Do not guess arbitrary ExecuteFunction overload payloads; collect exact metadata first.
- If thread context becomes the failure, fix the execution bridge rather than replacing the semantic AutoFight contract.
- DATA repo is reference-only for this task.

## Known-Good Summary
- Bridge Probe path: runtime PASS in v0.1.0.
- AutoFight ON/OFF: no known-good version yet.

## Important Technical Facts
- `C_AutoModel.None = 0`.
- `C_AutoModel.Train = 1`.
- `TopIcon.AutoTrainClick()` is the shipped zero-argument Train start wrapper.
- `TopIcon.AutoStopClick()` is the shipped zero-argument stop wrapper.
- `LuaSystemManager` exposes `ExecuteFunction`.
- `il2cpp_class_get_method_from_name(name, argc)` is insufficient to distinguish overloads with the same argc; v0.1.1 enumerates methods instead.

## Knowledge Index
- Feature: `docs/features/AUTO_FIGHT.md`
- v0.1.0: `docs/history/VERSION_v0.1.0.md`
- v0.1.1: `docs/history/VERSION_v0.1.1.md`
- Bugs: `docs/bugs/BUG_REGISTRY.md`
- Decisions: `docs/decisions/DECISIONS.md`
- Evidence: `docs/evidence/EVIDENCE_REGISTRY.md`
- Changelog: `CHANGELOG.md`
