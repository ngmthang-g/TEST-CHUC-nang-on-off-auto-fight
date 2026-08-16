# VERSION v0.1.0 — AutoFight semantic ON/OFF experiment

## A. Identity / Lineage
```text
Version: v0.1.0
Date: 2026-08-16
Based On: user-supplied ThanLongCleanRoute v1.0.0 source
Reason Created: isolate and test semantic built-in AutoFight Train ON/OFF
Last Known-Good AutoFight: UNKNOWN
Regression From: none established
Supersedes: none
Superseded By: none
```

## B. User Requests
- `REQ-001`: use supplied source as foundation.
- `REQ-002`: test ON/OFF AutoFight.
- `REQ-003`: DATA repo is reference/research only.
- `REQ-004`: keep engineering lineage/knowledge updated.

## C. State Before Modification
- Baseline source was a CleanRoute state/mount/path tool.
- Baseline build audit explicitly rejected AutoFight/combat tokens.
- Target GitHub test repo contained only its initial placeholder README.
- DATA knowledge already contained exact semantic Train start/stop facts, so no broad binary reverse was necessary.

## D. Investigation / Root Cause
Root Cause: `CONFIRMED` for the old visible-flow misconception.

The shipped client does not require `AUTO -> Đánh quái tab -> click` to start Train. The DATA KB records the real action as `AutoFight_Main:StartAutoFight(Train)` and the shipped zero-arg wrapper `TopIcon:AutoTrainClick()`.

External invocation mechanics are only `LIKELY / UNKNOWN` until live runtime proof.

## E. Changes Made

### Protocol
- Added `StartAutoFight`, `StopAutoFight`, and non-mutating `Probe` commands.
- Bumped protocol version.
- Isolated mapping prefix to the test tool.

### Controller
- Added only ON/OFF AutoFight controls to the visible test UI plus a bridge probe.
- Added explicit `RUNTIME UNTESTED` status wording.
- Logs bridge-returned diagnostic detail rather than claiming success from elapsed time.

### Bridge
- Resolves `LuaSystemManager` by metadata/class name.
- Resolves `ExecuteFunction` with narrow supported signatures.
- Supports static method or likely singleton instance lookup.
- Calls `TopIcon.AutoTrainClick` / `TopIcon.AutoStopClick`.
- Refuses action during map transition via a narrow safety guard.

## F. Important Implementation Details
```text
Start wrapper: TopIcon.AutoTrainClick
Stop wrapper:  TopIcon.AutoStopClick
Verified semantic target: AutoFight_Main.StartAutoFight(Train/None)
Train enum: 1
No mouse click emulation
No UIButton pointer replay
Runtime effect is not inferred from timeout/sleep
```

Current execution context remains inherited WH_GETMESSAGE hook callback context, not the desired production MainThread Action queue.

## G. Files / Components Changed
```text
Modified:
- src/protocol.h
- src/controller.cpp
- src/bridge.cpp
- resources/app.rc
- build.cmd

Added:
- src/controller_support.inl
- src/bridge_runtime.inl
- src/bridge_lua.inl
- .github/workflows/build.yml
- AI_INDEX.md
- PROJECT_KNOWLEDGE.md
- CHANGELOG.md
- docs/*
```

## H. Build / CI History
```text
Initial Build: NOT BUILT locally for Windows (Zig unavailable in local worker)
Local independent test: route_logic_test 8/8 PASS with g++
Final Build: PENDING GitHub Actions
CI: PENDING
Run: 31934886448 queued at knowledge-write time
Commit: b8dc4fc4475dc0fe9a42fc9d7513b9fa6c1cda9d
Artifact: PENDING
```

## I. Runtime Result
```text
RUNTIME: UNTESTED
Confirmed Working: none for AutoFight test yet
Still Failing: UNKNOWN
Awaiting Test: Train ON, Train OFF, stability
```

## J. Regression / Revert / Failed Attempts
No runtime attempt recorded yet.

## K. Known-Good Established
None. Do not mark known-good before live evidence.

## L. Remaining Bugs / New Knowledge / Decisions
- `BUG-001`: action thread boundary not canonical MainThread.
- `BUG-002`: live `ExecuteFunction` overload/instance compatibility unproven.
- `DEC-001`: use shipped semantic wrapper, not visual tab click.
- `DEC-002`: DATA repo remains read-only.

## M. Handoff
If v0.1.0 fails:
1. preserve exact tool log and whether the client crashed/disconnected/no-op;
2. if signature error, inspect only `LuaSystemManager.ExecuteFunction` metadata signature;
3. if managed/Lua exception, resolve script lifetime/current method name narrowly;
4. if crash/disconnect, move mutation to `MainThread.Execute(System.Action)` bridge;
5. do not broad reverse GameAssembly and do not replace the action with mouse macros.
