# Thần Long Item Consolidator v0.6

Direct base: exact v0.5 source. v0.6 adds the `AUTO PHÓ BẢN` tab immediately before `GIỚI THIỆU`; the original AUTO/World Flow/Sell/Trade/Rotation behavior remains in its old tab.

## AUTO PHÓ BẢN

- One active account and one dungeon profile at a time.
- Configurable NPC `ResID`, gathering map/position, expected dungeon MapID, entry/exit real-click sequences, loop mode and an ordered list of stages.
- Every stage has MapID, X/Y, tolerance, required kill count, filter group, optional count radius, timeout and BOSS marker.
- Live scanner lists dynamic RoleID, stable ResID when exposed, Name, HP/MaxHP, death state, position and runtime class/type.
- A scanned monster can be saved into an enabled filter group. ResID is preferred; exact name is the fallback only when the client does not expose ResID.
- Kill counting is `seen alive -> HP=0/dead`, deduplicated per dynamic RoleID/life. A first-seen corpse, repeated corpse scan, or disappearance from AOI is never counted. Seeing the same RoleID alive after a death rearms one new life.
- State machine: travel NPC -> `ClickNPC(ResID)` -> entry clicks -> prove dungeon MapID -> travel stage -> prove AutoFight ON -> scan/count -> prove AutoFight OFF -> next stage -> exit clicks -> prove map exit -> optional next loop.
- Timeouts are errors, never success evidence.

The template selector includes the 19 canonical rows from `database/FUBEN_SCENARIOS.csv` in the research repository. Templates fill known NPC/map information but intentionally leave combat coordinates at zero for the user to GET in the live dungeon.

## Mutual exclusion

Starting AUTO PHÓ BẢN while AUTO TRAIN is active asks to stop every running AUTO account. Starting AUTO TRAIN while a dungeon run is active asks to stop the dungeon run. Only one mode can own runtime actions.

## Build and test

The Windows x64 workflow runs:

1. `tools/verify_v06_logic.py`
2. Release EXE/DLL build
3. route, rotation, trade-coordinator and dungeon death-counter tests

`BUILD/CI PASS` is not `RUNTIME PASS`. The scanner RVAs and semantic AutoFight action are frozen-build guarded (`TimeDateStamp 0x6A410C14`, `SizeOfImage 0x03DCB000`) and still require a live test on that exact game client. Read `docs/AUTO_DUNGEON_RUNTIME_TEST_PLAN.md` before trusting an unattended loop.
