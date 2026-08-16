# AI INDEX — AutoFight Test

Repository: `ngmthang-g/TEST-CHUC-nang-on-off-auto-fight`

## Start here

Read in this order before changing code:

1. `AI_PROJECT_KNOWLEDGE_PROTOCOL_V2_OPTIMIZED.md`
2. `AI_CLIENT_ANALYSIS_RULES.txt`
3. `PROJECT_KNOWLEDGE.md`
4. `docs/features/AUTO_FIGHT.md`
5. `CHANGELOG.md`
6. `docs/history/VERSION_v0.1.0.md`
7. current source.

## Project scope

This repo exists only to test **semantic AutoFight Train ON/OFF** on the frozen Thần Long client.

Reference knowledge lives in `ngmthang-g/clinent-game-than-long-DATA-2222`. Treat that repo as read-only. Do not broad reverse-engineer the client when exact knowledge already exists there.

## Exact verified knowledge used by this test

```text
C_AutoModel.Train = 1
TopIcon:AutoTrainClick() -> AutoFight_Main:StartAutoFight(Train)
TopIcon:AutoStopClick()  -> AutoFight_Main:StartAutoFight(None)
```

The visible `Đánh quái` settings tab is not the semantic start command.

## Current implementation status

```text
Version: v0.1.0
Implementation: EXPERIMENTAL
Runtime: UNTESTED
Known-good AutoFight test version: UNKNOWN
```

## Current narrow questions

1. Does the current frozen client expose an `ExecuteFunction` overload compatible with the v0.1.0 adapter?
2. Is a live `LuaSystemManager` instance required and resolved correctly?
3. Does invoking the semantic wrapper through the inherited hook callback context work without crash/disconnect?
4. If not, move the mutable call to the canonical `MainThread.Execute(System.Action)` execution path; do not restart broad reverse engineering.
