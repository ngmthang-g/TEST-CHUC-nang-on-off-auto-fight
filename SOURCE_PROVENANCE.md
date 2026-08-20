# SOURCE PROVENANCE — v0.6

- Direct base: uploaded `ThanLongItemConsolidator_Source_v0.5(1).zip`, SHA-256 `d84b0f57a7da17e549c85e793232b37b5f8a518fe58f494d97fea4fd7393126d`.
- Nearby Name/RoleID/HP donor: uploaded `ThanLongAutoBuff_Source_v1.3.1(2).zip`, SHA-256 `4dedc55a9df08abd69632c79d67827cc8d5730272c7b9fe5f82551d7ce76ace4`. Only its exact frozen build metadata, ObjectManager layout and GRole getter RVAs are used; its CreateRemoteThread worker was not transplanted.
- Semantic AutoFight donor and build transport reference: `ngmthang-g/TEST-CHUC-nang-on-off-auto-fight`. The v0.6 bridge retains the existing WH_GETMESSAGE injection model and adds PE/signature plus Unity-main-thread proof before calling `AutoFight_Main.StartAutoFight`.
- Dungeon/NPC templates and API semantics: `ngmthang-g/clinent-game-than-long-DATA-2222`, especially `database/FUBEN_SCENARIOS.csv`, `AUTO_TOOL_API_CATALOG.md`, nearby schema and Auto Train/AutoFight documents.
- The `PMVL` CSV row contains `truetrue`; v0.6 treats the template only as map/NPC data and does not reproduce that malformed boolean.
- No undocumented MapID, NPC ResID or monster ResID was invented. Combat coordinates and monster filters are user-captured from the live game.
- Windows GitHub CI is build transport. Static/unit/build pass must not be reported as live scanner/FSM runtime pass.
