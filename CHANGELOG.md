# CHANGELOG

## [v0.1.1] - 2026-08-16

### Triggering runtime evidence
- User test: `Bridge probe PASS; chưa gửi AutoFight action`.
- User test: `ON FAIL: ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`.
- Conclusion: bridge Probe path works; AutoFight ON failed before Lua invocation at the `ExecuteFunction` resolver boundary.

### Changed / Fixed
- Replaced argc-only `il2cpp_class_get_method_from_name` selection for `ExecuteFunction` with full overload enumeration using `il2cpp_class_get_methods` and `il2cpp_method_get_name`.
- Added exact method matching by name + parameter count + every IL2CPP parameter type.
- Preserved fail-closed invocation policy: only `(String,String)` and `(String,String,Object[])` are invoked automatically.
- Added runtime metadata dump of every `ExecuteFunction` overload when neither supported shape exists.
- Bumped shared protocol to `0x00020100` so v0.1.0 EXE/DLL cannot be mixed with v0.1.1.
- Updated EXE/resource/artifact identity to v0.1.1.

### Runtime status
- v0.1.0 Bridge Probe: `PASS`.
- v0.1.0 AutoFight ON: `FAIL before Lua invoke`.
- v0.1.1 AutoFight ON/OFF: `RUNTIME UNTESTED`.
- No known-good AutoFight version yet.

### Next test
- If log reports `ExecuteFunction exact 2-param matched` or `exact 3-param matched`, verify actual ON/OFF behavior in game.
- If it reports `ExecuteFunction overloads: ...`, preserve the full line; that is the exact metadata required for the next narrow correction.
- Only investigate MainThread/thread context after a compatible `ExecuteFunction` method is actually invoked.

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
- Fixed Windows CI scope-audit UTF-8 handling by reading source with explicit `-Encoding UTF8`.

### Build
- Initial Windows CI failed only in UTF-8 scope audit before C++ compilation.
- Corrected Windows build/CI PASS: run `31935017087`, source/build commit `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0`.
- Scope audit PASS; route FSM regression `8/8 PASS`; bridge PE DLL validation PASS; controller EXE PASS.
- Artifact: `ThanLong-AutoFight-Test-v0.1.0`, artifact ID `9260424827`.

### Runtime result
- Bridge Probe: PASS.
- AutoFight ON: FAIL before Lua action because the adapter returned `ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`.
- This does not disprove the verified semantic AutoFight start/stop contract.
- MainThread/thread-context safety was not reached by this test because signature resolution failed first.
