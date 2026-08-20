# v0.6 implementation audit

## Requested

- Add AUTO PHÓ BẢN before GIỚI THIỆU.
- Scan and save nearby monster Name/IDs/HP.
- Count exact deaths per stage before moving to the next coordinate.
- Support boss completion, configurable entry/exit, looping and multiple editable profiles.
- Prevent AUTO TRAIN and AUTO PHÓ BẢN from running together.

## Implemented

- Versioned protocol `0x00010600`, 96 copied monster records, dedicated scan and semantic AutoFight commands.
- Frozen build/signature guard, ObjectManager AOI enumeration and GRole HP/MaxHP/Name getters from the supplied AutoBuff donor.
- Stage-epoch RoleID life tracker with unit tests for first-dead, duplicate-dead, respawn, group and reset behavior.
- Persisted profile/stage/filter/click-sequence UI plus 19 canonical dungeon templates.
- Fail-closed NPC -> entry -> stage -> boss -> exit -> loop FSM with authoritative map/position/AutoFight proofs.
- Two-way mutual exclusion confirmation.

## Deliberately not claimed

- Windows CI/build pass: pending until the source is pushed to the authorized build repository and Actions completes.
- Live scanner/AutoFight/FSM pass: pending the runtime test plan.
- Death by disappearance: deliberately not inferred.

## Protected

v0.5 AUTO tab behavior remains unchanged except for the explicit mutual-exclusion entry check and the branch that dispatches the one dungeon-owned account to the new FSM.
