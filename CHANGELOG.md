# CHANGELOG

## [v0.1.2] - 2026-08-16

### Triggering runtime evidence
v0.1.1 live client reported the exact `ExecuteFunction` overloads:
- `static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue`
- `static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue`

This disproved the old `(String,String)` / `(String,String,Object[])` adapter assumptions.

### Design correction
- Did not guess how `(String,Object[])` supplies Lua `self` for `TopIcon:AutoTrainClick()`.
- Preserved old runtime history showing direct `TopIcon.AutoTrainClick` dispatch could return without changing `EnableAutoF1`.
- Moved the test target to the lower-level persistent service:
  - ON: `AutoFight_Main.StartAutoFight(1)`.
  - OFF: `AutoFight_Main.StartAutoFight(0)`.

### Added / Changed
- Added `LuaSystemAPI_GUI.FindUI/MainFindUI("AutoFight_Main")` service lookup.
- Added Unity managed main-thread proof using `UnitySynchronizationContext` and managed thread IDs before mutation.
- Added exact frozen-client identity guard and byte-signature validation for the v0.9.0 MonoBehaviourExecutor donor entrypoints.
- Added boxed `System.Int32` mode and one-element `System.Object[]` construction with strong GC handles.
- Direct semantic call uses `MonoBehaviourExecutor.ExecuteUiObject(AutoFight_Main,"StartAutoFight",args)`.
- Added immediate `EnableAutoF1` diagnostic read when available.
- Bumped protocol to `0x00020200`.
- Updated EXE/resource/artifact identity to v0.1.2.

### Build / CI
- Run `31938163761`, head `e3a1241262aec66abe3195f2bc699eb4c1e1da32`: audit PASS, route regression 8/8 PASS, then FAILED because legacy v0.1.1 helper `AppendExecuteFunctionSignatures` became unused under `-Werror`.
- Fix head `1aed2b89f0d3aa1d8ea30774ac45078f872fd940`: explicitly retained the historical diagnostic helper.
- Corrected run `31938259381`: Build and audit PASS; artifact upload PASS.
- Artifact: `ThanLong-AutoFight-Test-v0.1.2`, ID `9261301912`, size `180271` bytes.
- Digest: `sha256:03d0bc330cde908a574ec0147876dea8310126313078ad44af73ea095e2ff7ca`.

### Runtime status
- v0.1.2 ON/OFF: `RUNTIME UNTESTED`.
- Known-good AutoFight effect remains `UNKNOWN`.
- CI success is not runtime success.

### Architecture caveat
v0.1.2 proves Unity managed main-thread identity but still calls the semantic action from the WH_GETMESSAGE callback. Production architecture should ultimately route the same action through a legitimate `System.Action -> FGStudio.Engine.Utilities.MainThread.Execute -> Unity Update` boundary if the direct callback path shows re-entrancy/lifecycle instability.

## [v0.1.1] - 2026-08-16

### Triggering runtime evidence
- v0.1.0: `Bridge probe PASS; chưa gửi AutoFight action`.
- v0.1.0 ON: `ExecuteFunction signature chưa hỗ trợ; cần metadata hẹp`.

### Changed / Fixed
- Replaced argc-only `ExecuteFunction` selection with full method enumeration and exact parameter matching.
- Added runtime metadata dump of every `ExecuteFunction` overload when the supported shapes did not exist.
- Protocol `0x00020100`.

### Build / CI
- PASS: run `31937194750`, head `3a90b6ca7e69e08d610491ae70231ceaf92ec1e6`.
- Artifact: `ThanLong-AutoFight-Test-v0.1.1`, ID `9261010029`, digest `sha256:3faa3b25be998c3aa65830228affc89a8efc095a84f3bacf158b93d7715dab12`.

### Runtime result
- Probe: PASS.
- ON: fail-closed metadata dump, no AutoFight mutation issued.
- Exact overloads verified:
  - `static(MoonSharp.Interpreter.Closure,System.Object[])->MoonSharp.Interpreter.DynValue`
  - `static(System.String,System.Object[])->MoonSharp.Interpreter.DynValue`
- v0.1.1 is therefore a successful metadata diagnostic version, not a known-good AutoFight action version.

## [v0.1.0] - 2026-08-16

### Requested
- Use the supplied CleanRoute source as foundation.
- Develop/test built-in AutoFight ON/OFF.
- Use the DATA repo only as reference/research.
- Preserve project knowledge/history per protocol.

### Implementation
- Dedicated AutoFight-only test UI.
- Shared-memory commands `StartAutoFight`, `StopAutoFight`, `Probe`.
- Initial `LuaSystemManager.ExecuteFunction` adapter around `TopIcon.AutoTrainClick/AutoStopClick`.
- No visual mouse macro.

### Build
- First CI failure: UTF-8 audit decoding only.
- Corrected CI PASS: run `31935017087`, head `4b50eadbaf2cb6ab6d0552a4d6d362aa51f72be0`.
- Artifact ID `9260424827`.

### Runtime result
- Bridge Probe: PASS.
- AutoFight ON: FAIL before Lua action at unsupported ExecuteFunction signature.
