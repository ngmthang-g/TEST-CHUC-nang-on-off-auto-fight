#pragma once

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <string>
#include <unordered_map>
#include <vector>

namespace cleanroute_dungeon {

struct MonsterObservation {
    std::int32_t roleID = 0;       // Dynamic instance identity for one spawned monster.
    std::int32_t resID = 0;        // Stable template identity when the client exposes it.
    std::int32_t hp = -1;
    std::int32_t maxHP = -1;
    std::int32_t x = 0;
    std::int32_t y = 0;
    bool dead = false;
    bool positionValid = false;
    std::wstring name;
    bool verifiedMonster = false;  // Exact GMonster class-chain proof from the bridge.
    bool liveVitalsValid = false;  // Live HP/MaxHP proof, never static Monsters config.
};

struct MonsterRule {
    std::wstring name;
    std::int32_t resID = 0;
    std::wstring group = L"THUONG";
    bool enabled = true;
    bool boss = false;
};

struct DungeonStage {
    std::wstring name;
    std::int32_t mapID = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t tolerance = 120;
    std::int32_t requiredKills = 40;
    std::int32_t countRadius = 0;  // 0 = the current nearby/AOI scan.
    std::int32_t timeoutSec = 900;
    std::wstring filterGroup = L"THUONG";
    bool boss = false;
};

struct KillEvent {
    std::int32_t roleID = 0;
    std::int32_t resID = 0;
    std::wstring name;
};

inline bool EqualFolded(const std::wstring& left, const std::wstring& right) {
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (std::towlower(left[i]) != std::towlower(right[i])) return false;
    }
    return true;
}

inline bool MatchesRule(const MonsterObservation& monster, const MonsterRule& rule,
                        const std::wstring& requiredGroup) {
    if (!rule.enabled) return false;
    if (!requiredGroup.empty() && !EqualFolded(rule.group, requiredGroup)) return false;
    if (rule.resID > 0 && monster.resID != rule.resID) return false;
    if (rule.resID <= 0 && !rule.name.empty() && !EqualFolded(monster.name, rule.name)) return false;
    return rule.resID > 0 || !rule.name.empty();
}

inline bool InCountRadius(const MonsterObservation& monster, std::int32_t centerX,
                          std::int32_t centerY, std::int32_t radius) {
    if (radius <= 0) return true;
    if (!monster.positionValid) return false;
    const std::int64_t dx = static_cast<std::int64_t>(monster.x) - centerX;
    const std::int64_t dy = static_cast<std::int64_t>(monster.y) - centerY;
    const std::int64_t rr = static_cast<std::int64_t>(radius) * radius;
    return dx * dx + dy * dy <= rr;
}

class DeathTracker {
public:
    void Reset(std::int32_t mapID = 0, std::int32_t stageIndex = -1) {
        lives_.clear();
        mapID_ = mapID;
        stageIndex_ = stageIndex;
    }

    std::vector<KillEvent> Observe(const std::vector<MonsterObservation>& scan,
                                   const std::vector<MonsterRule>& rules,
                                   const std::wstring& requiredGroup,
                                   std::int32_t centerX = 0,
                                   std::int32_t centerY = 0,
                                   std::int32_t radius = 0) {
        std::vector<KillEvent> events;
        for (const MonsterObservation& monster : scan) {
            if (!monster.verifiedMonster || !monster.liveVitalsValid ||
                monster.roleID <= 0 || monster.maxHP <= 0 || monster.hp < 0 ||
                monster.hp > monster.maxHP ||
                !InCountRadius(monster, centerX, centerY, radius)) {
                continue;
            }
            const auto matching = std::find_if(rules.begin(), rules.end(), [&](const MonsterRule& rule) {
                return MatchesRule(monster, rule, requiredGroup);
            });
            if (matching == rules.end()) continue;

            Life& life = lives_[monster.roleID];
            const bool aliveNow = !monster.dead && monster.hp > 0;
            const bool deadNow = monster.dead || monster.hp == 0;
            if (aliveNow) {
                // The same dynamic RoleID can be reused after a respawn. Seeing it alive
                // after a counted death explicitly rearms one new life.
                if (life.countedDead) life.countedDead = false;
                life.seenAlive = true;
                life.resID = monster.resID;
                continue;
            }
            if (deadNow && life.seenAlive && !life.countedDead) {
                // Fail closed: a first-seen corpse has no prior alive evidence and is never counted.
                life.countedDead = true;
                events.push_back({monster.roleID, monster.resID, monster.name});
            }
        }
        return events;
    }

    std::int32_t MapID() const { return mapID_; }
    std::int32_t StageIndex() const { return stageIndex_; }
    std::size_t TrackedLives() const { return lives_.size(); }

private:
    struct Life {
        bool seenAlive = false;
        bool countedDead = false;
        std::int32_t resID = 0;
    };

    std::unordered_map<std::int32_t, Life> lives_;
    std::int32_t mapID_ = 0;
    std::int32_t stageIndex_ = -1;
};

inline bool StageComplete(std::int32_t countedKills, std::int32_t requiredKills) {
    return requiredKills > 0 && countedKills >= requiredKills;
}

} // namespace cleanroute_dungeon
