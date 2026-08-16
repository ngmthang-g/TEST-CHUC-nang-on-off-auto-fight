# AutoFight test scope audit — v0.1.0

This repository evolved from the user-supplied CleanRoute source, but **the visible test scope is now only AutoFight Train ON/OFF**.

## Allowed test path

```text
scan GameAssembly.dll client
 -> attach inherited hook bridge
 -> map-transition safety check
 -> resolve LuaSystemManager
 -> ExecuteFunction
 -> TopIcon.AutoTrainClick / AutoStopClick
```

## Explicitly not part of this test

- mouse coordinates / `SendInput` / `mouse_event`;
- `UIButton.HandleClickEvent` replay;
- NPC interaction;
- selling;
- revive;
- target selection;
- skill casting;
- reconstructing combat;
- broad reverse engineering.

Inherited route code remains in the source tree as baseline/regression material, but its route controls are not exposed in the AutoFight test UI. The build continues to run the original pure route FSM self-test so baseline edits are detected.

## Runtime caveat

The current AutoFight invocation reuses the inherited hook callback execution context. This is **EXPERIMENTAL**, not the canonical production action engine. The reference KB's preferred mutable-action boundary remains:

```text
valid System.Action -> FGStudio.Engine.Utilities.MainThread.Execute -> Unity Update -> semantic action
```

Therefore:

```text
CI PASS != RUNTIME PASS
ExecuteFunction return without exception != AutoFight confirmed ON/OFF
```

Only live client behavior establishes the runtime result.
