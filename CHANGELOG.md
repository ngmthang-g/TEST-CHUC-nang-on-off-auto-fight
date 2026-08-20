# CHANGELOG

## v0.6.1 — 2026-08-20
- Corrected the v0.6 entity-category error: AutoBuff's nearby `GRole` player scanner is no longer treated as a monster scanner.
- Dungeon scan now accepts only objects whose runtime class chain contains exact `GMonster`; GRole-only players, NPCs, pets and other sprites are excluded before HP reads and before publication.
- Added independent counter barriers `MonsterValidClassProof` and `MonsterValidLiveVitals`; player-like or unproven records cannot arm a RoleID life even when a saved name/ResID rule matches.
- Live monster vitals prefer semantic `get_HP/get_MaxHP`; the frozen GRole RVA donor is a guarded fallback only when the accepted GMonster also proves a GRole base class.
- Added excluded-GRole/other-sprite/HP-failure diagnostics, explicit `HP:SEM`/`HP:RVA` UI provenance and regression tests for player/unproven-vitals rejection.
- Bumped protocol, product, workflow and artifact identity to v0.6.1. Windows build and live GMonster/HP/death proof remain pending.

## v0.6 — 2026-08-20
- Added `AUTO PHÓ BẢN` before `GIỚI THIỆU`, with persisted profiles, 19 research-repo templates, editable NPC/map settings, stages, monster filters and entry/exit real-click sequences.
- Added the first frozen-build-guarded nearby HP scanner. Its broad `Role`/`Monster` class-name acceptance could include players and is superseded by v0.6.1 strict GMonster proof.
- Added stage-scoped death counter: only observed alive -> HP=0/dead counts; repeated corpses, first-seen corpses and AOI disappearance do not count; respawn rearms one new life.
- Added fail-closed dungeon FSM from NPC travel through map entry, ordered kill stages, boss, exit and optional looping. Map/state/AutoFight are proven at every transition; timeouts stop as errors.
- Added semantic `AutoFight_Main.StartAutoFight(Train=1/None=0)` bridge command with Unity-main-thread and exact PE/signature guards; controller verifies the resulting snapshot state.
- Added hard mutual exclusion prompts between AUTO TRAIN and AUTO PHÓ BẢN.
- Added `dungeon_logic_tests`, `verify_v06_logic.py`, feature/runtime-test documentation and updated CI artifact naming.
- Preserved v0.5 World Flow/P1/P2/P3, User Mouse Guard, Sell/Trade/Rotation and route rules outside the explicit mutual-exclusion integration.

## v0.5 — 2026-08-20
- Fixed World Flow/BĐPT HOLD so held MAIN/CON accounts are movement-observed before P1/P2/P3 instead of losing the Lâu Lan stall watchdog.
- Fixed held-account death lifecycle: P2 Đầu thai preempts World Flow; travel/rendezvous coordinator pauses without dropping FIFO/HOLD, then held travel restarts cleanly after ALIVE. Atomic trade click Sequence safety is unchanged.
- Preserved tool-owned cross-map route ownership/evidence across intermediate MapID changes, so entering Lâu Lan no longer disarms the gate watchdog.
- Tightened Lâu Lan condition to require current AutoPath=ON plus >=3s position stall.
- Removed the v0.4 background `PostMessageW` XN implementation from active runtime. P1 XN now foregrounds the target game, moves the real cursor and clicks through `SendInput`, borrowing/restoring any existing click-sequence lease for exactly one click.
- User Mouse Guard 5s, periodic-XN removal, route/click separation, Travel Guard, mount recovery, Adaptive Sell Step 5 and protected Sell/Trade/Rotation rules retained.

## v0.4 — 2026-08-20
- Removed scheduler-wide click freeze: physical click sequences keep only a click lease; bridge AutoPath/Mount/Dismount continue in parallel between clicks.
- Removed periodic `Xác nhận ra Map mỗi N giây` runtime/config/UI while preserving Confirm coordinate/capture/manual test.
- Added Lâu Lan-only (`MapID=5`) cross-map stall watchdog: after observed AutoPath movement, >=3s position stall triggers a hidden/background Confirm `PostMessageW` click.
- Added global User Mouse Guard: physical mouse activity pauses all automatic clicks for 5s from the latest input without pausing scanner/FSM/route/mount.
- Sell/Trade/P3 retain exact click step/request if mouse activity is detected during click preparation.
- v0.3 AutoFight Travel Guard, mount recovery, Adaptive Sell Step 5, Sell/Trade/Rotation business rules retained.

## v0.3 — 2026-08-20
- Added one fail-closed AutoFight Travel Guard before every StartPath.
- Removed duplicated/fail-open stop-Auto movement logic from Sell, Train Recovery, Trade Rendezvous and M87.
- Added global P3 AUTO UI barrier below XN and Đầu thai; P3 preserves SELL/GD sequence lease state.
- Restored requested mount recovery: Mount x2 -> fight 10s -> stop -> Mount x2 -> foot 15s -> repeat.
- Kept Adaptive Sell Step 5 and unrelated business logic unchanged.
- Added living `TOOL_LOGIC_TABLE.html`.

## Historical changelog from pre-v0.3 source
 — HISTORICAL RECORD

> This file intentionally mentions old behavior from old versions. It is **not** the current runtime specification. For current R13 behavior, read `PROJECT_KNOWLEDGE.md` and `README.md`.

## v0.2.7-R6
- SELL click sequence is atomic relative to trade workflow: trade coordinator yields for the entire sell macro and final delay.
- SELL acquires its sequence lease immediately after successful ClickNPC.
- Added `AUTO` / `GIỚI THIỆU` tab navigation.
- About text: `Thiết kế và phát triển bởi Thắng Nguyễn - ĐỒ LONG`.
- R5 trade target/FIFO and all protected behaviors retained.

## v0.2.7-R6 — 2026-08-19

### Requested / changed
- Each CON now has a per-account `GD đến trống ≥ N` target, default 30 and persisted by profile.
- A FULL CON still enters the trade workflow only at 0 free slots, but after each complete shared trade-click sequence the same CON remains active and repeats the entire sequence while `FreeBagSpace < N`. It is released back to normal train only when `FreeBagSpace >= N`.
- The final click delay is honored before evaluating the bag snapshot, reducing stale post-confirm reads.
- Simultaneous FULL admissions are deterministic by child number (CON1 before CON2 ...); after admission the queue remains strict FIFO by workflow-entry time, so an already-staged CON3 stays ahead of a later CON1.

### Protected
- R4 queue max 3, one trade sequence at a time, AutoPath freeze exemption, F8 capture fix, F4, two raw SendInput sites, MAIN sell threshold 6, FULL-only entry threshold 0, drag/group/rotation/train/sell/revive/route logic remain unchanged outside the requested integration points.

### Verification
- Local R5 static scope audit: PASS.
- Windows GitHub CI: pending at package-prep stage.
- Runtime: **RUNTIME UNTESTED** until live trade test.

## v0.2.7-R4 — 2026-08-19

### Requested / changed
- Stage up to 3 FULL children at TỌA GD while keeping the actual trade click sequence strictly sequential.
- Long Freeze All now begins only when MAIN + active child are at TỌA GD and the trade click sequence starts; periodic Auto XN map remains available during travel/wait.
- Mouse-drag selects contiguous trade-sequence rows.
- Rotation is disabled with only the combo-selected map and activates only when a second map is manually checked; combo selection resets the checked pool.

### Protected
- F4 byte-identical to R3, one F4 registration, two raw SendInput call sites, MAIN threshold 6, child FULL threshold 0.
- Existing DỒN ĐỒ, R3 multi-delete, group repeat, REC/sell/train/route/revive and unrelated core behavior remain outside scope.
- No v0.2.8/v0.2.9 code imported.

### Verification
- Local exact patch reproduction + static audit: PASS.
- GitHub Windows CI workflow committed, but completed CI run not observable when archive finalized; CI PASS not claimed.
- Runtime: **RUNTIME UNTESTED**.

## v0.2.7-R3 — 2026-08-17

### Requested
- Only change the trade click-sequence editor so `- XÓA` deletes all currently selected rows instead of only one focused row.
- Keep every other runtime feature/code path unchanged.

### Changed
- `DeleteTradeSequenceRow()` now uses the editor's existing multi-selection (`SelectedRows`).
- Selected rows are deleted from highest index to lowest index.
- When deleting multiple rows from `CHUỖI GD MAIN`, shared ACC CON `MAIN #n` references are repaired across all removed MAIN rows.
- Single-row delete remains supported by the same code path.

### Protected / Unchanged
- F4, DỒN ĐỒ, TỌA GD rendezvous, group repeat, AutoFight fallback, five-click model, BĐPT/REAL INPUT, sell/REC/train/route/revive logic.
- No v0.2.8/v0.2.9 code imported.

### Source
- R2 base controller: `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`.
- R3 controller: `a69fa0df4932e4020aed6e61b4109bd2c558db5c407afedb46c03456fb575abf`.
- R3 patch: `3b013821934c882cce8dc755894f66ab835feec394d3433015127a8792fc2136`.
- CI #60 / run `32048547405`: FAILED before compile because Windows checkout changed patch transport EOL; runtime source not implicated.
- CI #61 / run `32048648947`: **CI PASS / BUILD PASS** including rehydrate, x64 build, route/rotation/trade tests, static multi-delete-only audit and artifact upload.
- Runtime status: **RUNTIME UNTESTED** until live multi-delete is tested.

## v0.2.7-R2 — 2026-08-17

### Requested
- Verify `DỒN ĐỒ: BẬT/TẮT` actually gates consolidation.
- FULL CON immediately holds MAIN+CON and sends both to one user-captured TỌA GD.
- First arrival waits; both must be confirmed at TỌA GD before existing trade clicks.
- Add grouped mini-sequence repetition for one or more consecutive trade rows.
- Remove all per-CON selector coordinates.
- AutoFight-stop failure must not deadlock movement; retry after map transition.
- Merge old AUTO + DỪNG AUTO 1 into one `AUTO` point.
- Preserve F4 exactly.

### Added / Changed / Fixed
- Added global TỌA GD capture/persistence and dedicated `tradeTravel*` rendezvous state.
- Both transaction participants are `tradeHeld` immediately on FULL CON selection.
- Added first-arrival hold/StopPath behavior and both-arrived readiness gate.
- Added group metadata/editor/runtime loop while preserving row repeat.
- Removed `tradeSelectPoint`, `TradeSelect*` persistence/UI/runtime.
- Per-account click array changed 6→5; `AUTO` replaces old duplicated StopAuto1 point.
- Train/sell/M87/trade movement can continue after bounded AutoFight-stop retries and retry after map change.
- `ReleaseTradeHolds()` now cleans all rendezvous holds/state.
- Restored untouched clean-v0.2.7 `PeriodicConfirmBusy()` and `HandleFightClicks()` after CI detected they had been accidentally deleted during the first edit pass.

### Build / CI
- Local clean-base patch chain: PASS, final controller `de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2`.
- CI #55: FAILED before build due Windows EOL checksum mismatch; corrected by LF normalization.
- CI #56: rehydrate/configure PASS, BUILD FAILED due accidental removal of two untouched clean helpers; corrected by byte-exact restoration.
- CI #57 / run `32043612053`, job `95427111307`: **CI PASS**, including Windows x64 Build Release + route/rotation/trade tests + static requested-only audit + artifact upload.
- Runtime remains **RUNTIME UNTESTED**; CI does not upgrade it.

### Runtime
- Status: **RUNTIME UNTESTED**.
- Protected evidence: F4 behavior from clean v0.2.7 is user-confirmed useful; exact pause block is byte-protected.

### Next Version Notes
- First collect live evidence for rendezvous/map transitions/group repeat/DỒN OFF/F4 before redesigning any state machine.

## v0.2.7
- Replaced per-CON trade workflows with one global `CHUỖI GD ACC CON` shared by CON1..CON6.
- Runtime binds CON-targeted rows to active transaction child and retains shared MAIN references.
- DỒN ĐỒ ON/OFF, BĐPT, REC, sell FREEZE and donor core were preserved.

## v0.2.6
- Added `DỒN ĐỒ: BẬT/TẮT` and independent auto-train/sell while OFF.
- Added whole sell-sequence cloning.

## v0.2.7-R7
- AUTO XN MAP becomes global highest-priority automatic physical click via scheduler pre-pass; interrupted SELL/TRADE/AUTO click state resumes immediately after the XN click with no XN post-delay.
- Added immutable workflow-entry tickets and Rendezvous FIFO relock so an earlier workflow child cannot be overtaken by a later traveler; same-batch ties preserve lower CON number first.
- Preserved R6/R5 behavior outside this scope.

## v0.2.7-R8
- XN MAP is now global-pass-only and acts as an all-window auto-click barrier.
- Only the due/eligible target game window receives its own Confirm coordinate.
- A window currently inside SELL/TRADE/AUTO UI click sequence defers its own XN; no Confirm injection into that busy UI.
- Other windows resume their existing sequence state immediately after the XN click.

## v0.2.7-R9 — Đầu thai global barrier cấp 2
- Giữ nguyên toàn bộ R8 ngoài đúng click Đầu thai.
- XN map vẫn ưu tiên cấp 1.
- Đầu thai trở thành ưu tiên cấp 2 toàn cục: pause mọi auto click cửa sổ khác, click đúng cửa sổ acc chết, rồi resume ngay.
- Death-session được prime trong global pre-pass để không bị sequence lease của cửa sổ khác làm chậm phát hiện chết.
- Giữ nguyên 500ms / 5000ms / 900ms / 4500ms của flow Đầu thai cũ.

## v0.2.7-R11
- Removed hidden MaxTransferClicks INI cap from active trade runtime; legacy key is normalized to 90 only for compatibility.
- Disabled hidden scheduled shutdown and clear stale ShutdownEnabled at startup.
- Added bounded MAIN FreeBagSpace stabilization after each complete trade pass before applying R10 <=8 received-slot heuristic.
- Trade sequence now requires at least one real CON CHUYỂN ĐỒ row to be READY.
- Protected R10/R9/R8/R7/R6 behavior unchanged outside these four runtime-hardening points.

## v0.2.7-R12
- Removed dead `CON GD đến trống ≥ N` UI/profile setting.
- DỒN ĐỒ OFF now respects each account's Auto Sell checkbox; DỒN ĐỒ ON MAIN/CON sell semantics remain coordinator-owned.
- Removed legacy trade macro subsystem, dead shutdown/confirm/transfer-cap state and obsolete TradeTxn fields.
- Cleaned bridge protocol baggage and bumped protocol to 0x00010503.
- Moved historical source snapshots and macro files out of active `src/`.
- Replaced detached toy trade test with shared runtime coordinator decision helpers + real unit coverage.
## v0.2.7-R13 — strict source cleanup + Auto Sell master switch
- Auto Sell checkbox is authoritative in every mode: unchecked blocks automatic selling; checked permits existing thresholds/role rules.
- Removed the remaining legacy `.macro` archive and legacy trade-macro header from the distributable source package.
- Removed duplicate historical controller/base-source implementations from the distributable package; Git/patch lineage remains the history source.
- Old version notes/manifests moved under `history/version-notes/`; current root identity is R13 only.
- Re-audited absence of dead shutdown, fixed-CON-target, legacy trade-macro runtime and childTrigger UI symbols.
