# AI INDEX — AutoFight Test

Repository: `ngmthang-g/TEST-CHUC-nang-on-off-auto-fight`

## Start here
Read in this order before changing code:

1. `AI_PROJECT_KNOWLEDGE_PROTOCOL_V2_OPTIMIZED.md`
2. `AI_CLIENT_ANALYSIS_RULES.txt`
3. `PROJECT_KNOWLEDGE.md`
4. `docs/features/AUTO_FIGHT.md`
5. `docs/bugs/BUG_REGISTRY.md`
6. `docs/evidence/EVIDENCE_REGISTRY.md`
7. `CHANGELOG.md`
8. `docs/history/VERSION_v0.1.2.md`
9. current source.

## Project scope
This repo exists only to test **semantic AutoFight Train ON/OFF** on the frozen Thần Long client.

Reference knowledge lives in `ngmthang-g/clinent-game-than-long-DATA-2222`. Treat that repo as read-only. Do not broad reverse-engineer the client when exact knowledge already exists there.

## Exact verified semantic knowledge
```text
C_AutoModel.None  = 0
C_AutoModel.Train = 1
TopIcon:AutoTrainClick() -> AutoFight_Main:StartAutoFight(Train)
TopIcon:AutoStopClick()  -> AutoFight_Main:StartAutoFight(None)
Preferred lower-level service target -> AutoFight_Main:StartAutoFight(mode)
```

The visible `Đánh quái` settings tab is not the start command.

## Exact runtime metadata now verified
v0.1.1 live client proved `LuaSystemManager.ExecuteFunction` has exactly these observed overloads:

```text
static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue
static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue
```

Do not rediscover these signatures unless the client build changes.

## Current implementation status
```text
Version: v0.1.2
Implementation: EXPERIMENTAL DIRECT SERVICE TEST
Build/CI: PASS
Runtime ON/OFF: UNTESTED
Bridge Probe: RUNTIME PASS from prior versions
Known-good AutoFight effect: UNKNOWN
```

## v0.1.2 action path
```text
WH_GETMESSAGE bridge
 -> map safety guards
 -> UnitySynchronizationContext + managed main-thread ID proof
 -> exact frozen client + donor byte-signature validation
 -> FindUI/MainFindUI("AutoFight_Main")
 -> MonoBehaviourExecutor.ExecuteUiObject("StartAutoFight", [boxed Int32 mode])
 -> immediate EnableAutoF1 diagnostic when available
```

## Current narrow questions
1. Does v0.1.2 ON actually start built-in Train in the live client?
2. Does v0.1.2 OFF actually stop it?
3. Does direct ExecuteUiObject from the proven Unity-main-thread WH_GETMESSAGE callback remain stable, or does normal-Update re-entrancy matter?
4. If instability appears after semantic correctness is proven, preserve the semantic action and move execution behind legitimate `System.Action -> FGStudio.Engine.Utilities.MainThread.Execute -> Unity Update`.

## Do not do
- Do not return to visual mouse clicking.
- Do not use `CreateRemoteThread` gameplay workers.
- Do not prefer `TopIcon.AutoTrainClick` over `AutoFight_Main.StartAutoFight` without new evidence.
- Do not mark BUILD/CI as runtime PASS.
