# EVIDENCE REGISTRY

## EVID-001
Type: SOURCE
Date / Version: 2026-08-16 / v0.1.0
Source: user-supplied `ThanLongCleanRoute_v1.0.0_FIXED_GitHubActions` source archive.
Observation: provides working project structure, WH_GETMESSAGE injection/bridge pattern, IL2CPP runtime resolver patterns, build script and route regression test.
Supports: baseline source lineage for v0.1.0.
Does NOT Prove: AutoFight action runtime success.
Confidence: CONFIRMED.

## EVID-002
Type: REVERSE_ENGINEERING / SOURCE
Date / Version: reference KB current at 2026-08-16
Source: reference DATA repo, especially `features/AUTO_TRAIN.md`, `analysis/10_BUILTIN_AUTO_FIGHT_ENGINE.md`, `analysis/13_UI_RUNTIME_ACTION_SURFACE.md`.
Observation: `Train=1`, `None=0`; `AutoFight_Main:StartAutoFight(mode)` is the underlying semantic service; `TopIcon.AutoTrainClick/AutoStopClick` are wrappers.
Supports: semantic ON/OFF target.
Does NOT Prove: this test bridge can invoke it safely.
Confidence: CONFIRMED for client semantics.

## EVID-003
Type: TEST
Date / Version: 2026-08-16 / baseline
Source: inherited route FSM self-test.
Observation: 8/8 regression cases PASS.
Supports: baseline route logic has not been accidentally broken.
Does NOT Prove: AutoFight runtime.
Confidence: CONFIRMED.

## EVID-004 — Initial v0.1.0 Windows CI failure
Type: CI / LOG
Date / Version: 2026-08-16 / v0.1.0
Source: GitHub Actions run `31934886448`, commit `b8dc4fc4475dc0fe9a42fc9d7513b9fa6c1cda9d`.
Observation: CI failed in scope audit because PowerShell did not explicitly decode UTF-8 Vietnamese controller text.
Supports: root cause of initial pipeline failure.
Does NOT Prove: AutoFight C++ or runtime failure.
Confidence: CONFIRMED.

## EVID-005 — Corrected v0.1.0 Windows CI/build/artifact PASS
Type: CI / BUILD
Date / Version: 2026-08-16 / v0.1.0
Source: GitHub Actions run `31935017087`, head `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0`.
Observation: scope audit PASS; route FSM 8/8 PASS; bridge DLL build/PE validation PASS; controller EXE PASS; artifact `ThanLong-AutoFight-Test-v0.1.0` ID `9260424827`, digest `sha256:f716e2a33eba0a9eea7cc889b9ab19d3f4ea4c1c10db19856a732f1167223598`.
Does NOT Prove: live AutoFight effect.
Confidence: CONFIRMED for build/CI.

## EVID-006 — v0.1.0 live bridge Probe PASS / adapter FAIL before action
Type: USER RUNTIME LOG
Date / Version: 2026-08-16 / v0.1.0
Source: user live-client test.
Observation:
- `Bridge probe PASS; chưa gửi AutoFight action`
- `ON FAIL: ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`
Supports: controller/shared memory/hook/protocol reached in-process bridge; mutation was not invoked.
Does NOT Prove: AutoFight semantic success/failure or mutation thread safety.
Confidence: CONFIRMED.

## EVID-007 — v0.1.1 exact overload resolver
Type: SOURCE / DIAGNOSTIC DESIGN
Date / Version: 2026-08-16 / v0.1.1
Observation: full `ExecuteFunction` overload enumeration by exact method name and parameter types; unsupported signatures are dumped instead of guessed.
Supports: narrow runtime metadata collection.
Does NOT Prove: action success.
Confidence: CONFIRMED for source change.

## EVID-008 — v0.1.1 Windows CI/build/artifact PASS
Type: CI / BUILD
Date / Version: 2026-08-16 / v0.1.1
Source: GitHub Actions run `31937194750`, head `3a90b6ca7e69e08d610491ae70231ceaf92ec1e6`.
Observation: Build/audit and artifact upload PASS; artifact ID `9261010029`, digest `sha256:3faa3b25be998c3aa65830228affc89a8efc095a84f3bacf158b93d7715dab12`.
Does NOT Prove: live ON/OFF.
Confidence: CONFIRMED for CI/build.

## EVID-009 — v0.1.1 live exact ExecuteFunction metadata
Type: USER RUNTIME LOG
Date / Version: 2026-08-16 / v0.1.1
Source: user live-client test, PID 4324.
Observation:
- `Bridge probe PASS; chưa gửi AutoFight action`
- `ON FAIL: ExecuteFunction overloads: static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue | static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue`
Supports:
- exact live overload set is now known;
- v0.1.0 assumed signatures were wrong for this client;
- v0.1.1 failed closed before AutoFight mutation, as designed.
Does NOT Prove:
- the string overload's Lua function-name convention for colon methods/self;
- AutoFight Train can or cannot start;
- mutable thread safety.
Confidence: RUNTIME VERIFIED for the overload metadata.

## EVID-010 — prior donor history favors direct AutoFight_Main service over TopIcon wrapper
Type: SOURCE-INSPECTED DONOR / PRIOR RUNTIME HISTORY
Date / Version: historical AutoTrain v0.8.4-v0.9.0; used to design test v0.1.2
Source: preserved AutoTrain source/patches for the same frozen client plus DATA semantic documents.
Observation:
- v0.8.4 direct `TopIcon.AutoTrainClick` dispatch did not change `EnableAutoF1`.
- Lua source shows `TopIcon:AutoTrainClick()` calls `AutoFight_Main:StartAutoFight(Train)` and also `self:ShowAutoStatus(Train)`.
- later donor code invoked `AutoFight_Main.StartAutoFight(1/0)` directly through the UI/Lua executor.
Supports: choosing persistent `AutoFight_Main.StartAutoFight(Train/None)` as the narrower v0.1.2 semantic target.
Does NOT Prove: the new WH_GETMESSAGE bridge path will succeed live.
Confidence: CONFIRMED donor/history evidence; new runtime effect UNVERIFIED.

## EVID-011 — v0.1.2 first CI failure is legacy unused-helper only
Type: CI / LOG
Date / Version: 2026-08-16 / v0.1.2
Source: GitHub Actions run `31938163761`, head `e3a1241262aec66abe3195f2bc699eb4c1e1da32`.
Observation:
- new v0.1.2 semantic/main-thread/donor scope audit PASS;
- route FSM regression 8/8 PASS;
- bridge compile stopped at `-Werror,-Wunused-function` because `AppendExecuteFunctionSignatures` from v0.1.1 was retained but no longer referenced.
Supports: first v0.1.2 CI failure was a source-cleanliness warning caused by superseding the old adapter, not a reported type/ABI compile error in the new action bridge.
Does NOT Prove: runtime action success.
Confidence: CONFIRMED.

## EVID-012 — corrected v0.1.2 Windows CI/build/artifact PASS
Type: CI / BUILD
Date / Version: 2026-08-16 / v0.1.2
Source: GitHub Actions run `31938259381`, head `1aed2b89f0d3aa1d8ea30774ac45078f872fd940`.
Observation:
- Build and audit PASS under `-Wall -Wextra -Werror`;
- artifact upload PASS;
- artifact `ThanLong-AutoFight-Test-v0.1.2`;
- artifact ID `9261301912`, size `180271` bytes;
- digest `sha256:03d0bc330cde908a574ec0147876dea8310126313078ad44af73ea095e2ff7ca`.
Supports: v0.1.2 compiles/packages with direct `AutoFight_Main.StartAutoFight(1/0)`, Unity-main-thread proof, frozen-client/byte-signature guards, boxing/Object[] construction and GC rooting code.
Does NOT Prove: live ON/OFF behavior, disconnect/crash safety, or production `MainThread.Execute(System.Action)` compliance.
Confidence: CONFIRMED for build/CI; RUNTIME UNVERIFIED.
