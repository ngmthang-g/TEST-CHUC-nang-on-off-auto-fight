# AUTO PHÓ BẢN — v0.6.1

## User workflow

1. Select an account and create/duplicate a dungeon profile.
2. Fill NPC ResID, GET its current map/X/Y, and set the expected dungeon MapID.
3. Record the NPC dialog under `CHUỖI VÀO`; record any manual exit flow under `CHUỖI RA`.
4. Add/remove/reorder stage coordinates. GET each coordinate in the live dungeon and set required kills plus its filter group.
5. Use `QUÉT MONSTER / HP`. Only rows with exact `GMonster` class-chain proof are shown; inspect Name/RoleID/ResID/live HP, add the intended monsters, then assign normal or boss groups.
6. Start the run. If AUTO TRAIN is active, the tool asks before stopping it.

## Identity and counter contract

- `GMonster` class-chain proof is mandatory. GRole-only players, NPCs, pets and other objects are excluded and cannot arm the counter.
- ResID identifies a monster template and is the preferred persisted filter.
- RoleID identifies one dynamic nearby instance/life and is the death-deduplication key.
- A stage epoch begins only after position arrival and authoritative AutoFight ON.
- Only live-vitals-proven `GMonster: alive (HP>0) -> HP=0/IsDeath` emits exactly one event.
- First-seen corpses, repeated corpses and AOI disappearance emit no event.
- Seeing the same RoleID alive again rearms a new life.
- Map or stage transition clears all prior life evidence.
- A positive radius requires valid monster position. Radius 0 counts matching objects in the current ObjectManager/AOI scan.

## State proofs

| Transition | Required proof |
| --- | --- |
| Travel | map ready, no map transition, authoritative AutoFight OFF |
| NPC arrived | expected map and X/Y within tolerance, AutoPath OFF |
| Entered | exact configured dungeon MapID |
| Stage arrived | exact stage MapID and X/Y within tolerance, AutoPath OFF |
| Fighting | authoritative AutoFight ON |
| Stage complete | death counter >= configured requirement |
| Leaving stage | authoritative AutoFight OFF |
| Exited | current MapID differs from dungeon MapID |

Every timeout or missing proof is fail-closed.

## Bridge boundary

The scanner and semantic AutoFight action run only inside the existing WH_GETMESSAGE bridge callback. Monster classification uses exact class ancestry, never a class-name substring. Exact RVAs are guarded by the frozen PE tuple and byte signatures. The controller stores only copied values in shared memory; it never retains client object pointers.

## Known runtime risk

If the server removes a monster before any 500 ms scan observes HP=0/death, that kill is intentionally missed rather than inferred from disappearance. Live testing should determine whether a shorter safe scan interval or an authoritative death-event donor is needed.
