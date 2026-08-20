# v0.6 verification status — 2026-08-20

| Gate | Status |
| --- | --- |
| v0.6 static contract | PASS locally + GitHub Actions |
| Route logic tests | 15/15 PASS locally + GitHub Actions |
| Rotation logic tests | 8/8 PASS locally + GitHub Actions |
| Trade coordinator tests | PASS locally + GitHub Actions |
| Dungeon death-counter tests | PASS locally + GitHub Actions |
| Windows x64 MSVC compile | PASS — Actions run 32385342052, commit cd125e9 |
| Live nearby scanner | RUNTIME UNTESTED |
| Live semantic AutoFight ON/OFF | RUNTIME UNTESTED |
| Full dungeon loop | RUNTIME UNTESTED |

GitHub Actions produced the Windows x64 EXE/DLL artifact successfully. Build/CI PASS is not live-game runtime proof; the three runtime gates above remain mandatory.
