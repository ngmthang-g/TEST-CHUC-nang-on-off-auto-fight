# CHANGELOG

## [v0.1.0] - 2026-08-16

### Requested
- `REQ-001`: use the supplied CleanRoute source as the foundation.
- `REQ-002`: develop and test turning AutoFight/Đánh quái ON and OFF.
- `REQ-003`: use `clinent-game-than-long-DATA-2222` only as reference/research data.
- `REQ-004`: preserve project knowledge/history per the supplied AI project protocol.

### Added / Changed / Fixed
- Added protocol commands `StartAutoFight` and `StopAutoFight`.
- Added a dedicated AutoFight-only test UI.
- Added `LuaSystemManager.ExecuteFunction` adapter for shipped `TopIcon.AutoTrainClick` / `TopIcon.AutoStopClick` wrappers.
- Isolated test shared-memory mapping and bridge DLL naming from CleanRoute.
- Split bridge/controller helpers into small `.inl` modules to keep the test surface narrow.
- Added GitHub Actions Windows/Zig build.
- Added project knowledge/history/feature/bug/decision/evidence documentation.

### Removed / Reverted
- Route/spot controls are no longer exposed in the AutoFight test UI.
- No visual mouse-click AutoFight path was added.

### Files / Modules
- Modified: `src/protocol.h`, `src/controller.cpp`, `src/bridge.cpp`, `resources/app.rc`, `build.cmd`.
- Added: `src/controller_support.inl`, `src/bridge_runtime.inl`, `src/bridge_lua.inl`, `.github/workflows/build.yml`, `AI_INDEX.md`, `PROJECT_KNOWLEDGE.md`, protocol/rule files and `docs/` knowledge tree.

### Build
- Local route-logic regression test: `8/8 PASS` using host `g++`.
- Windows Zig build: pending GitHub Actions for source commit `b8dc4fc4475dc0fe9a42fc9d7513b9fa6c1cda9d`.
- CI: pending at this knowledge commit time.

### Runtime
- Status: `RUNTIME UNTESTED`.
- Confirmed working: none yet for AutoFight v0.1.0.
- Awaiting test: actual Train ON, actual Train OFF, crash/disconnect/no-op behavior.

### Regression / Known-Good / Related Bugs
- Last known-good AutoFight test: `UNKNOWN`.
- `BUG-001`, `BUG-002` remain open implementation risks until runtime evidence.

### Next Version Notes
- If runtime fails, collect exact bridge log first.
- Fix only the narrow `ExecuteFunction` signature/manager/thread boundary indicated by evidence.
- Prefer migration to the canonical MainThread Action dispatcher rather than adding click emulation.
