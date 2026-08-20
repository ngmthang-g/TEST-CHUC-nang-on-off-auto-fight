#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def text(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


required = {
    "src/protocol.h": [
        "kProtocolVersion = 0x00010600u",
        "ScanNearbyMonsters",
        "SetAutoFight",
        "MonsterRecord",
        "kMaxMonsterRecords = 96",
    ],
    "src/bridge.cpp": [
        "ValidateFrozenBuild",
        "ObjectManagerGetInstanceRva = 0x655010u",
        "GRoleGetHPRva = 0x69B0B0u",
        "GRoleGetMaxHPRva = 0x69C780u",
        "GRoleGetNameRva = 0x69D080u",
        "AutoFight_Main",
        "StartAutoFight",
        "UnitySynchronizationContext",
        "case Command::ScanNearbyMonsters",
        "case Command::SetAutoFight",
    ],
    "src/dungeon_logic.h": [
        "life.seenAlive",
        "!life.countedDead",
        "monster.hp == 0",
        "first-seen",
        "void Reset",
    ],
    "src/dungeon_app_methods.inl": [
        "ClickNpc",
        "WaitDungeonMap",
        "TravelStage",
        "StartFight",
        "Fighting",
        "StopFight",
        "WaitExitMap",
        "StageComplete",
        "Timeout",
        "CHUỖI VÀO",
        "CHUỖI RA",
        "Chỉ một trong hai tab được phép chạy",
    ],
    "src/controller.cpp": [
        'L"AUTO PHÓ BẢN"',
        "TickDungeonAccount(a, now)",
        "StopDungeonInternal(L\"chuyển sang AUTO TRAIN\"",
        "captureDungeonSequenceIndex_",
    ],
    "CMakeLists.txt": ["VERSION 0.6.0", "dungeon_logic_tests"],
    ".github/workflows/build.yml": ["verify_v06_logic.py", "dungeon_logic_tests.exe"],
}

errors: list[str] = []
for filename, needles in required.items():
    body = text(filename)
    for needle in needles:
        if needle not in body:
            errors.append(f"{filename}: missing {needle!r}")

combined = "\n".join(text(name) for name in ["src/controller.cpp", "src/bridge.cpp"])
for forbidden in ["CreateRemoteThread", "WriteProcessMemory", "VirtualAllocEx"]:
    if forbidden in combined:
        errors.append(f"active v0.6 source must not use {forbidden}")

if errors:
    print("v0.6 verification FAILED")
    for error in errors:
        print(" -", error)
    sys.exit(1)

print("v0.6 static contract PASS")
print("NOTE: BUILD PASS is not RUNTIME PASS; live game scanner/AutoFight/FSM test remains mandatory.")
