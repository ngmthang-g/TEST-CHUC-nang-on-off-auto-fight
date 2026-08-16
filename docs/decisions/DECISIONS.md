# DECISIONS

## DEC-001
Date / Version: 2026-08-16 / v0.1.0
Status: ACTIVE
Decision: Invoke shipped semantic `TopIcon.AutoTrainClick` / `TopIcon.AutoStopClick` wrappers rather than reproducing screen clicks.
Context: DATA KB proves the visible `Đánh quái` tab is settings only and identifies exact Train start/stop semantics.
Alternatives: click visible tab/button; replay UIButton; rebuild combat actions.
Why Rejected: visually fragile, semantically wrong, or unnecessary.
Evidence: DATA `features/AUTO_TRAIN.md`, `analysis/10_BUILTIN_AUTO_FIGHT_ENGINE.md`.
Reason: smallest correct test surface.
Consequences: external tool must solve Lua execution context rather than UI coordinates.
Affected Features: AUTO_FIGHT.
Superseded By: none.

## DEC-002
Date / Version: 2026-08-16 / v0.1.0
Status: ACTIVE
Decision: `clinent-game-than-long-DATA-2222` is read-only reference material for this test repo.
Context: user explicitly separated test code repo from research/data repo.
Alternatives: commit experimental code into DATA repo.
Why Rejected: mixes implementation and canonical frozen-client knowledge.
Evidence: user request.
Reason: protect research corpus and keep experiment isolated.
Consequences: all source/build/history changes land only in TEST-CHUC-nang-on-off-auto-fight.
Affected Features: whole project.
Superseded By: none.

## DEC-003
Date / Version: 2026-08-16 / v0.1.0
Status: ACTIVE
Decision: isolate mapping/DLL/executable names from the CleanRoute baseline.
Context: both tools may target the same PID during development.
Alternatives: reuse `Local\\ThanLongCleanRoute_*` and CleanRoute binary names.
Why Rejected: collision and operator confusion risk.
Evidence: source inspection.
Reason: test isolation.
Consequences: `Local\\ThanLongAutoFightTest_*`, `ThanLongAutoFightTestBridge.dll`, `ThanLongAutoFightTest_v0.1.0.exe`.
Affected Features: bridge/controller.
Superseded By: none.
