# Thần Long Item Consolidator v0.6.1

Direct base: v0.6 AUTO PHÓ BẢN. v0.6.1 corrects the entity boundary: the AutoBuff player scanner remains a separate donor, while dungeon scanning accepts only objects whose runtime class chain contains the exact class `GMonster`.

## AUTO PHÓ BẢN

- One active account and one dungeon profile at a time.
- Configurable NPC `ResID`, gathering map/position, expected dungeon MapID, entry/exit real-click sequences, loop mode and an ordered list of stages.
- Every stage has MapID, X/Y, tolerance, required kill count, filter group, optional count radius, timeout and BOSS marker.
- Live monster scanner excludes GRole-only players, NPCs, pets and other sprite families before reading/counting. It lists only exact `GMonster` class-chain matches with dynamic RoleID, stable ResID when exposed, Name, live HP/MaxHP, death state, position and runtime class/type.
- A scanned monster can be saved into an enabled filter group. ResID is preferred; exact name is the fallback only when the client does not expose ResID.
- Kill counting additionally requires `GMonster class proof + live-vitals proof`, then applies `seen alive -> HP=0/dead`, deduplicated per dynamic RoleID/life. A player can never arm the counter even if its RoleID/name/HP happens to match a saved rule.
- State machine: travel NPC -> `ClickNPC(ResID)` -> entry clicks -> prove dungeon MapID -> travel stage -> prove AutoFight ON -> scan/count -> prove AutoFight OFF -> next stage -> exit clicks -> prove map exit -> optional next loop.
- Timeouts are errors, never success evidence.

The template selector includes the 19 canonical rows from `database/FUBEN_SCENARIOS.csv` in the research repository. Templates fill known NPC/map information but intentionally leave combat coordinates at zero for the user to GET in the live dungeon.

## Mutual exclusion

Starting AUTO PHÓ BẢN while AUTO TRAIN is active asks to stop every running AUTO account. Starting AUTO TRAIN while a dungeon run is active asks to stop the dungeon run. Only one mode can own runtime actions.

## Build and test

The Windows x64 workflow runs:

1. `tools/verify_v061_logic.py`
2. Release EXE/DLL build
3. route, rotation, trade-coordinator and dungeon death-counter tests

`BUILD/CI PASS` is not `RUNTIME PASS`. The scanner RVAs and semantic AutoFight action are frozen-build guarded (`TimeDateStamp 0x6A410C14`, `SizeOfImage 0x03DCB000`) and still require a live test on that exact game client. Read `docs/AUTO_DUNGEON_RUNTIME_TEST_PLAN.md` before trusting an unattended loop.
