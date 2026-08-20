# PROJECT KNOWLEDGE — v0.6.1 CURRENT

## v0.6.1 strict monster correction

- AutoBuff scans nearby player `GRole` objects. It is a donor for safe ObjectManager traversal only; it is not a monster identity source.
- Dungeon scanning accepts an object only when its actual runtime class or a parent class is exactly `GMonster`. Substring matching such as `Role`/`Monster` is prohibited.
- GRole-only players, NPCs, pets and other sprites are excluded before they reach the dungeon record list or death tracker.
- The death tracker independently requires `MonsterValidClassProof` and `MonsterValidLiveVitals`; this second barrier prevents a UI/protocol regression from counting a person.
- Live HP uses semantic `get_HP/get_MaxHP` first. The frozen GRole donor RVA is allowed only when the accepted GMonster also proves a `GRole` base class.
- Static Monsters data may interpret ResID but never supplies current HP or death proof.

## v0.6 AUTO PHÓ BẢN invariants retained

- Tabs are `AUTO`, `AUTO PHÓ BẢN`, `GIỚI THIỆU`. AUTO TRAIN and AUTO PHÓ BẢN are mutually exclusive and require an explicit user confirmation before the other mode is stopped.
- NPC selection uses `LuaSystemAPI_Game.ClickNPC(int)` with **GNPC ResID**, never dynamic RoleID.
- Monster filters persist stable `ResID + Name + Group`; death deduplication uses the dynamic RoleID for one observed life.
- A kill requires prior alive evidence in the same stage epoch followed by `HP == 0` or `IsDeath`. First-seen dead, repeated dead and disappearance are fail-closed non-events. Map/stage transition resets all life evidence.
- Stage completion requires the configured counter. A timeout, scan failure, map mismatch, character death or unreadable authoritative state stops the workflow as an error.
- Every movement action first proves AutoFight OFF. Every fight stage proves AutoFight ON; each stage transition proves it OFF again.
- `StartAutoFight(Train=1/None=0)` uses the frozen semantic `AutoFight_Main` donor only after PE/signature and Unity-main-thread proof. A command return is not success; the next `Snapshot.autoFight` is authoritative.
- Nearby monster scanner runs inside the existing WH_GETMESSAGE bridge on the game message thread. It does not use CreateRemoteThread, WriteProcessMemory or external pointer retention.
- Build/CI status and live runtime status are separate. v0.6.1 remains runtime-untested until the checklist in `docs/AUTO_DUNGEON_RUNTIME_TEST_PLAN.md` is completed.

## User rules
- Version sequence: 0.3, 0.4, ..., 0.9, 1.0, 1.1, ..., 1.9, 2.0...
- Do not change unrelated existing logic without prior approval.
- If a new feature replaces an old feature, remove the old active runtime logic completely, not only its UI.
- Every change must include conflict analysis and update `TOOL_LOGIC_TABLE.html`.

## v0.5 World Flow priority rule
`tradeHeld`/BĐPT World Flow is a workflow hold, **not an observer freeze**. Snapshot + movement observation runs for every active account before the priority passes. A held account still gets:
1. P1 Lâu Lan gate XN when eligible.
2. P2 Đầu thai when dead.
3. P3 AUTO UI when requested and safe.

During travel/rendezvous, a dead/reviving held account keeps its FIFO/HOLD entry. The trade coordinator pauses rather than releasing/aborting that staged World Flow. Only the immutable FIFO ticket is preserved through the hard life-boundary runtime reset; travel phases restart cleanly after ALIVE. The already-started atomic trade click Sequence retains its older fail-closed abort/safety semantics.

## v0.5 Lâu Lan gate watchdog
- `MapID 5 = Lâu Lan`.
- Old timer-driven periodic XN remains removed.
- Tool-owned cross-map route ownership/evidence now survives intermediate MapID transitions.
- Due condition requires: Map 5 + tool-owned cross-map route + route has progressed + current `AutoPath=ON` + no movement for >=3000 ms.
- P1 uses **real physical input**: foreground + `SetCursorPos` + `SendInput`; it intentionally occupies the mouse and leaves the cursor at the XN point.
- P1 may temporarily borrow an existing SELL/GD sequence lease for exactly one click, then restores the old lease owner/state.
- Active runtime contains **no `PostMessageW` background XN path**.

## User Mouse Guard retained
A `WH_MOUSE_LL` hook records non-injected physical mouse movement/buttons/wheel. Every automatic click waits until 5000 ms have elapsed since the latest physical input. Scanner/state observer/AutoPath/Mount/Dismount are not globally paused.

## Protected rules retained
Every `Command::StartPath` remains fail-closed behind the authoritative AutoFight Travel Guard. Mount recovery remains Mount x2 -> fight 10s -> stop AutoFight -> Mount x2 -> foot AutoPath 15s -> StopPath -> repeat. Adaptive Sell Step 5, Auto Sell thresholds, trade FIFO/relock/pass/bag stabilization, rotation, F4/F8, recorder/profile semantics are unchanged except the explicit World Flow death-recovery integration above.
