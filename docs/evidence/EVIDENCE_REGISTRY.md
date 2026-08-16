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

## EVID-004 — Initial Windows CI failure
Type: CI / LOG
Date / Version: 2026-08-16 / v0.1.0
Source: GitHub Actions run `31934886448`, commit `b8dc4fc4475dc0fe9a42fc9d7513b9fa6c1cda9d`.
Observation: `CI FAILED` in the AutoFight scope audit with `Controller missing BẬT AUTO FIGHT`; the failure happened before C++ compilation. The audit used PowerShell source reads without explicit UTF-8 decoding, so Vietnamese source text was not matched correctly.
Supports: root cause of the first CI failure is the audit text-decoding layer, not demonstrated AutoFight C++ compile failure.
Does NOT Prove: the AutoFight runtime adapter works.
Confidence: CONFIRMED.
Limitations: this evidence is only about the failed build pipeline attempt.

## EVID-005 — Corrected Windows CI/build/artifact pass
Type: CI / BUILD
Date / Version: 2026-08-16 / v0.1.0
Source: GitHub Actions run `31935017087`, source/build commit `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0`.
Observation:
- AutoFight semantic scope audit PASS.
- inherited route FSM regression test `8/8 PASS`.
- AutoFight bridge DLL compiled successfully.
- bridge passed PE DLL validation with characteristics `0x2022`.
- controller EXE compiled successfully.
- artifact upload PASS: `ThanLong-AutoFight-Test-v0.1.0`, artifact ID `9260424827`, size `116272` bytes.
- artifact digest: `sha256:f716e2a33eba0a9eea7cc889b9ab19d3f4ea4c1c10db19856a732f1167223598`.
Supports: `BUILD PASS` and `CI PASS` for the v0.1.0 test source at commit `4b50eadb...`.
Does NOT Prove: AutoFight actually turns ON/OFF in the live client, or that the current hook callback thread is a safe production mutation context.
Confidence: CONFIRMED.
Limitations: GitHub CI cannot execute the private/live game runtime and therefore cannot establish `RUNTIME PASS` or `KNOWN-GOOD`.
