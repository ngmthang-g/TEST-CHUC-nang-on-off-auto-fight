#include "dungeon_logic.h"

#include <cassert>
#include <vector>

using namespace cleanroute_dungeon;

int main() {
    DeathTracker tracker;
    const std::vector<MonsterRule> rules = {
        {L"Sơn Tặc", 501, L"THUONG", true, false},
        {L"Đại Vương", 999, L"BOSS", true, true},
    };

    tracker.Reset(92, 0);
    assert(tracker.Observe({{10, 501, 0, 100, 0, 0, true, false, L"Sơn Tặc"}},
                           rules, L"THUONG").empty()); // first-seen corpse is not a kill
    assert(tracker.Observe({{11, 501, 80, 100, 0, 0, false, false, L"Sơn Tặc"}},
                           rules, L"THUONG").empty());
    assert(tracker.Observe({{11, 501, 0, 100, 0, 0, true, false, L"Sơn Tặc"}},
                           rules, L"THUONG").size() == 1);
    assert(tracker.Observe({{11, 501, 0, 100, 0, 0, true, false, L"Sơn Tặc"}},
                           rules, L"THUONG").empty()); // same corpse is deduplicated

    assert(tracker.Observe({{11, 501, 100, 100, 0, 0, false, false, L"Sơn Tặc"}},
                           rules, L"THUONG").empty()); // respawn rearms the life
    assert(tracker.Observe({{11, 501, 0, 100, 0, 0, false, false, L"Sơn Tặc"}},
                           rules, L"THUONG").size() == 1);

    assert(tracker.Observe({{12, 999, 1, 1000, 0, 0, false, false, L"Đại Vương"}},
                           rules, L"THUONG").empty()); // wrong group cannot arm/count
    assert(tracker.Observe({{12, 999, 0, 1000, 0, 0, true, false, L"Đại Vương"}},
                           rules, L"BOSS").empty()); // corpse still was not armed in BOSS group
    assert(tracker.Observe({{12, 999, 1, 1000, 0, 0, false, false, L"Đại Vương"}},
                           rules, L"BOSS").empty());
    assert(tracker.Observe({{12, 999, 0, 1000, 0, 0, true, false, L"Đại Vương"}},
                           rules, L"BOSS").size() == 1);

    tracker.Reset(92, 1);
    assert(tracker.Observe({{11, 501, 0, 100, 0, 0, true, false, L"Sơn Tặc"}},
                           rules, L"THUONG").empty()); // stage reset drops old life evidence

    tracker.Reset(92, 2);
    assert(tracker.Observe({{20, 501, 10, 100, 500, 500, false, true, L"Sơn Tặc"}},
                           rules, L"THUONG", 0, 0, 100).empty());
    assert(tracker.Observe({{20, 501, 0, 100, 500, 500, true, true, L"Sơn Tặc"}},
                           rules, L"THUONG", 0, 0, 100).empty()); // outside radius
    assert(StageComplete(40, 40));
    assert(!StageComplete(39, 40));
    return 0;
}
