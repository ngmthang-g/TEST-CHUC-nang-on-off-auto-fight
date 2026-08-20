#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def text(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


required = {
    "src/protocol.h": [
        "kProtocolVersion = 0x00010601u",
        "ScanNearbyMonsters",
        "MonsterValidClassProof",
        "MonsterValidLiveVitals",
        "MonsterHpSource",
        "excludedPlayerRoles",
        "monsterHpReadFailures",
    ],
    "src/bridge.cpp": [
        "ValidateFrozenBuild",
        "ClassChainHasExactName",
        'ClassChainHasExactName(actual, "GMonster")',
        'ClassChainHasExactName(actual, "GRole")',
        "MonsterHpSource::SemanticGetter",
        "MonsterHpSource::GuardedGRoleSubclassRva",
        '"get_MonsterID"',
        '"monsterResID"',
        "SCAN STRICT GMonster=",
        "case Command::ScanNearbyMonsters",
        "AutoFight_Main",
        "UnitySynchronizationContext",
    ],
    "src/dungeon_logic.h": [
        "monster.verifiedMonster",
        "monster.liveVitalsValid",
        "life.seenAlive",
        "!life.countedDead",
        "monster.hp == 0",
        "first-seen",
    ],
    "src/dungeon_logic_test.cpp": [
        "a player/GRole can never arm the monster counter",
        "class proof without live HP proof is still rejected",
    ],
    "src/dungeon_app_methods.inl": [
        "QUÉT MONSTER / HP",
        "Chỉ nhận class GMonster",
        "requiredProof = MonsterValidClassProof | MonsterValidIdentity",
        "MonsterValidLiveVitals",
        "observation.verifiedMonster",
        "observation.liveVitalsValid",
        "ClickNpc",
        "WaitDungeonMap",
        "StageComplete",
        "Chỉ một trong hai tab được phép chạy",
    ],
    "src/controller.cpp": [
        'L"AUTO PHÓ BẢN"',
        "TickDungeonAccount(a, now)",
        'L"Thần Long Item Consolidator v0.6.1',
    ],
    "CMakeLists.txt": ["VERSION 0.6.1", "dungeon_logic_tests"],
    ".github/workflows/build.yml": ["verify_v061_logic.py", "dungeon_logic_tests.exe"],
}

errors: list[str] = []
for filename, needles in required.items():
    body = text(filename)
    for needle in needles:
        if needle not in body:
            errors.append(f"{filename}: missing {needle!r}")

bridge = text("src/bridge.cpp")
if "ContainsAscii(className" in bridge:
    errors.append("bridge scanner must not classify monsters by class-name substring")

combined = "\n".join(text(name) for name in ["src/controller.cpp", "src/bridge.cpp"])
for forbidden in ["CreateRemoteThread", "WriteProcessMemory", "VirtualAllocEx"]:
    if forbidden in combined:
        errors.append(f"active v0.6.1 source must not use {forbidden}")

if errors:
    print("v0.6.1 verification FAILED")
    for error in errors:
        print(" -", error)
    sys.exit(1)

print("v0.6.1 strict monster contract PASS")
print("NOTE: BUILD PASS is not RUNTIME PASS; live GMonster/HP/death proof remains mandatory.")
