# EVIDENCE REGISTRY

## EVID-001
Type: SOURCE
Date / Version: 2026-08-16 / v0.1.0
Source: user-supplied `ThanLongCleanRoute_v1.0.0_FIXED_GitHubActions` source archive.
Observation: provides working project structure, WH_GETMESSAGE injection/bridge pattern, IL2CPP runtime resolver patterns, build script and route regression test.
Supports: baseline source lineage for v0.1.0.
Does NOT Prove: AutoFight action runtime success.
Confidence: CONFIRMED.
Limitations: source/build artifact presence is not current live runtime evidence.

## EVID-002
Type: REVERSE_ENGINEERING / SOURCE
Date / Version: reference KB current at 2026-08-16 / v0.1.0
Source: `ngmthang-g/clinent-game-than-long-DATA-2222`, especially `features/AUTO_TRAIN.md`, `contexts/BUILD_AUTO_TRAIN.md`, `analysis/10_BUILTIN_AUTO_FIGHT_ENGINE.md`.
Observation: `Train=1`; shipped `TopIcon.AutoTrainClick()` starts Train via `AutoFight_Main.StartAutoFight(Train)`; `TopIcon.AutoStopClick()` stops with `None`.
Supports: semantic ON/OFF target selected by v0.1.0.
Does NOT Prove: this external bridge can invoke it safely.
Confidence: CONFIRMED for client semantics.
Limitations: external invocation is a separate implementation boundary.

## EVID-003
Type: TEST
Date / Version: 2026-08-16 / v0.1.0
Source: local `g++ -std=c++17 -Wall -Wextra -Werror src/route_logic_test.cpp`.
Observation: 8/8 inherited route FSM regression cases PASS.
Supports: baseline route logic was not accidentally broken by repository preparation.
Does NOT Prove: Windows controller/bridge build or AutoFight runtime.
Confidence: CONFIRMED.
Limitations: host-side pure C++ test only.

## EVID-004
Type: CI
Date / Version: 2026-08-16 / v0.1.0
Source: GitHub Actions run `31934886448` for source commit `b8dc4fc4475dc0fe9a42fc9d7513b9fa6c1cda9d`.
Observation: QUEUED at knowledge-write time.
Supports: pending Windows/Zig compile + audit + artifact validation.
Does NOT Prove: runtime success even if CI passes.
Confidence: UNKNOWN until run completes.
Limitations: CI cannot exercise the live game client.
