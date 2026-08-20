    static DungeonProfile MakeDungeonProfile(const std::wstring& name, int dungeonMapID = 0,
                                              int gatherMapID = 0, int npcResID = 0,
                                              int npcX = 0, int npcY = 0) {
        DungeonProfile profile{};
        profile.name = name.empty() ? L"PHÓ BẢN MỚI" : name;
        profile.dungeonMapID = dungeonMapID;
        profile.gatherMapID = gatherMapID;
        profile.npcResID = npcResID;
        profile.npcX = npcX;
        profile.npcY = npcY;
        profile.stages = {
            {L"TỌA 1", dungeonMapID, 0, 0, 120, 40, 0, 900, L"THUONG", false},
            {L"TỌA 2", dungeonMapID, 0, 0, 120, 40, 0, 900, L"THUONG", false},
            {L"BOSS", dungeonMapID, 0, 0, 120, 1, 0, 900, L"BOSS", true},
        };
        return profile;
    }

    static int ClampDungeonInt(int value, int low, int high, int fallback) {
        if (value < low || value > high) return fallback;
        return value;
    }

    void LoadDungeonProfiles() {
        dungeonProfiles_.clear();
        int count = ReadIniInt(L"DungeonGlobal", L"ProfileCount", 0);
        count = std::clamp(count, 0, 64);
        for (int i = 0; i < count; ++i) {
            const std::wstring section = L"DungeonProfile_" + std::to_wstring(i);
            DungeonProfile p{};
            p.name = ReadIniText(section, L"Name");
            p.gatherMapID = ReadIniInt(section, L"GatherMap", 0);
            p.npcResID = ReadIniInt(section, L"NpcResID", 0);
            p.npcX = ReadIniInt(section, L"NpcX", 0);
            p.npcY = ReadIniInt(section, L"NpcY", 0);
            p.dungeonMapID = ReadIniInt(section, L"DungeonMap", 0);
            p.entryTimeoutSec = ClampDungeonInt(ReadIniInt(section, L"EntryTimeout", 45), 5, 600, 45);
            p.exitTimeoutSec = ClampDungeonInt(ReadIniInt(section, L"ExitTimeout", 90), 5, 1800, 90);
            p.loop = ReadIniInt(section, L"Loop", 1) != 0;

            const int stageCount = std::clamp(ReadIniInt(section, L"StageCount", 0), 0, 64);
            for (int n = 0; n < stageCount; ++n) {
                const std::wstring prefix = L"Stage_" + std::to_wstring(n) + L"_";
                DungeonStage stage{};
                stage.name = ReadIniText(section, prefix + L"Name");
                stage.mapID = ReadIniInt(section, prefix + L"Map", p.dungeonMapID);
                stage.x = ReadIniInt(section, prefix + L"X", 0);
                stage.y = ReadIniInt(section, prefix + L"Y", 0);
                stage.tolerance = ClampDungeonInt(ReadIniInt(section, prefix + L"Tolerance", 120), 20, 5000, 120);
                stage.requiredKills = ClampDungeonInt(ReadIniInt(section, prefix + L"Kills", 40), 1, 100000, 40);
                stage.countRadius = ClampDungeonInt(ReadIniInt(section, prefix + L"Radius", 0), 0, 100000, 0);
                stage.timeoutSec = ClampDungeonInt(ReadIniInt(section, prefix + L"Timeout", 900), 10, 86400, 900);
                stage.filterGroup = ReadIniText(section, prefix + L"Group");
                stage.boss = ReadIniInt(section, prefix + L"Boss", 0) != 0;
                if (stage.name.empty()) stage.name = L"TỌA " + std::to_wstring(n + 1);
                if (stage.filterGroup.empty()) stage.filterGroup = stage.boss ? L"BOSS" : L"THUONG";
                p.stages.push_back(std::move(stage));
            }

            const int ruleCount = std::clamp(ReadIniInt(section, L"RuleCount", 0), 0, 256);
            for (int n = 0; n < ruleCount; ++n) {
                const std::wstring prefix = L"Rule_" + std::to_wstring(n) + L"_";
                MonsterRule rule{};
                rule.name = ReadIniText(section, prefix + L"Name");
                rule.resID = ReadIniInt(section, prefix + L"ResID", 0);
                rule.group = ReadIniText(section, prefix + L"Group");
                rule.enabled = ReadIniInt(section, prefix + L"Enabled", 1) != 0;
                rule.boss = ReadIniInt(section, prefix + L"Boss", 0) != 0;
                if (rule.group.empty()) rule.group = rule.boss ? L"BOSS" : L"THUONG";
                if (rule.resID > 0 || !rule.name.empty()) p.rules.push_back(std::move(rule));
            }

            auto loadClicks = [&](const wchar_t* key, std::vector<DungeonClickStep>& destination) {
                const int stepCount = std::clamp(ReadIniInt(section, std::wstring(key) + L"Count", 0), 0, 64);
                for (int n = 0; n < stepCount; ++n) {
                    const std::wstring prefix = std::wstring(key) + L"_" + std::to_wstring(n) + L"_";
                    DungeonClickStep step{};
                    step.description = ReadIniText(section, prefix + L"Desc");
                    step.point.x = ReadIniInt(section, prefix + L"X", -1);
                    step.point.y = ReadIniInt(section, prefix + L"Y", -1);
                    step.point.baseW = ReadIniInt(section, prefix + L"W", 0);
                    step.point.baseH = ReadIniInt(section, prefix + L"H", 0);
                    step.point.valid = step.point.x >= 0 && step.point.y >= 0 &&
                                       step.point.baseW > 0 && step.point.baseH > 0;
                    step.delayMs = ClampDungeonInt(ReadIniInt(section, prefix + L"Delay", 600), 50, 60000, 600);
                    step.repeat = ClampDungeonInt(ReadIniInt(section, prefix + L"Repeat", 1), 1, 999, 1);
                    destination.push_back(std::move(step));
                }
            };
            loadClicks(L"Entry", p.entryClicks);
            loadClicks(L"Exit", p.exitClicks);
            if (p.name.empty()) p.name = L"PHÓ BẢN " + std::to_wstring(i + 1);
            if (p.stages.empty()) p.stages = MakeDungeonProfile(p.name, p.dungeonMapID).stages;
            dungeonProfiles_.push_back(std::move(p));
        }
        if (dungeonProfiles_.empty()) dungeonProfiles_.push_back(MakeDungeonProfile(L"PHÓ BẢN MẪU"));
        dungeonProfileIndex_ = std::clamp(dungeonProfileIndex_, 0, static_cast<int>(dungeonProfiles_.size()) - 1);
    }

    void SaveDungeonProfiles() {
        EnsureUnicodeIni();
        const int oldCount = std::clamp(ReadIniInt(L"DungeonGlobal", L"ProfileCount", 0), 0, 128);
        const int clearCount = std::max(oldCount, static_cast<int>(dungeonProfiles_.size()));
        for (int i = 0; i < clearCount; ++i) {
            const std::wstring section = L"DungeonProfile_" + std::to_wstring(i);
            (void)WritePrivateProfileStringW(section.c_str(), nullptr, nullptr, ConfigPath().c_str());
        }
        WriteIniInt(L"DungeonGlobal", L"ProfileCount", static_cast<int>(dungeonProfiles_.size()));
        for (std::size_t i = 0; i < dungeonProfiles_.size(); ++i) {
            const DungeonProfile& p = dungeonProfiles_[i];
            const std::wstring section = L"DungeonProfile_" + std::to_wstring(i);
            WriteIniText(section, L"Name", p.name);
            WriteIniInt(section, L"GatherMap", p.gatherMapID);
            WriteIniInt(section, L"NpcResID", p.npcResID);
            WriteIniInt(section, L"NpcX", p.npcX);
            WriteIniInt(section, L"NpcY", p.npcY);
            WriteIniInt(section, L"DungeonMap", p.dungeonMapID);
            WriteIniInt(section, L"EntryTimeout", p.entryTimeoutSec);
            WriteIniInt(section, L"ExitTimeout", p.exitTimeoutSec);
            WriteIniInt(section, L"Loop", p.loop ? 1 : 0);
            WriteIniInt(section, L"StageCount", static_cast<int>(p.stages.size()));
            for (std::size_t n = 0; n < p.stages.size(); ++n) {
                const DungeonStage& stage = p.stages[n];
                const std::wstring prefix = L"Stage_" + std::to_wstring(n) + L"_";
                WriteIniText(section, prefix + L"Name", stage.name);
                WriteIniInt(section, prefix + L"Map", stage.mapID);
                WriteIniInt(section, prefix + L"X", stage.x);
                WriteIniInt(section, prefix + L"Y", stage.y);
                WriteIniInt(section, prefix + L"Tolerance", stage.tolerance);
                WriteIniInt(section, prefix + L"Kills", stage.requiredKills);
                WriteIniInt(section, prefix + L"Radius", stage.countRadius);
                WriteIniInt(section, prefix + L"Timeout", stage.timeoutSec);
                WriteIniText(section, prefix + L"Group", stage.filterGroup);
                WriteIniInt(section, prefix + L"Boss", stage.boss ? 1 : 0);
            }
            WriteIniInt(section, L"RuleCount", static_cast<int>(p.rules.size()));
            for (std::size_t n = 0; n < p.rules.size(); ++n) {
                const MonsterRule& rule = p.rules[n];
                const std::wstring prefix = L"Rule_" + std::to_wstring(n) + L"_";
                WriteIniText(section, prefix + L"Name", rule.name);
                WriteIniInt(section, prefix + L"ResID", rule.resID);
                WriteIniText(section, prefix + L"Group", rule.group);
                WriteIniInt(section, prefix + L"Enabled", rule.enabled ? 1 : 0);
                WriteIniInt(section, prefix + L"Boss", rule.boss ? 1 : 0);
            }
            auto saveClicks = [&](const wchar_t* key, const std::vector<DungeonClickStep>& source) {
                WriteIniInt(section, std::wstring(key) + L"Count", static_cast<int>(source.size()));
                for (std::size_t n = 0; n < source.size(); ++n) {
                    const DungeonClickStep& step = source[n];
                    const std::wstring prefix = std::wstring(key) + L"_" + std::to_wstring(n) + L"_";
                    WriteIniText(section, prefix + L"Desc", step.description);
                    WriteIniInt(section, prefix + L"X", step.point.valid ? step.point.x : -1);
                    WriteIniInt(section, prefix + L"Y", step.point.valid ? step.point.y : -1);
                    WriteIniInt(section, prefix + L"W", step.point.valid ? step.point.baseW : 0);
                    WriteIniInt(section, prefix + L"H", step.point.valid ? step.point.baseH : 0);
                    WriteIniInt(section, prefix + L"Delay", step.delayMs);
                    WriteIniInt(section, prefix + L"Repeat", step.repeat);
                }
            };
            saveClicks(L"Entry", p.entryClicks);
            saveClicks(L"Exit", p.exitClicks);
        }
        FlushIni();
    }

    HWND DungeonMake(const wchar_t* cls, const wchar_t* text, DWORD style,
                     int x, int y, int w, int h, int id) {
        HWND handle = Make(cls, text, style, x, y, w, h, id);
        if (handle) SendMessageW(handle, WM_SETFONT,
                                 reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
        dungeonControls_.push_back(handle);
        return handle;
    }

    void DungeonListColumn(HWND list, int index, int width, const wchar_t* text) {
        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = const_cast<wchar_t*>(text);
        column.cx = width;
        column.iSubItem = index;
        ListView_InsertColumn(list, index, &column);
    }

    void BuildDungeonUi() {
        DungeonMake(L"STATIC", L"Acc:", SS_LEFT | SS_CENTERIMAGE, 18, 42, 38, 27, 0);
        dungeonAccountCombo_ = DungeonMake(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL,
                                           58, 42, 205, 250, IDC_DG_ACCOUNT);
        DungeonMake(L"STATIC", L"Hồ sơ:", SS_LEFT | SS_CENTERIMAGE, 272, 42, 48, 27, 0);
        dungeonProfileCombo_ = DungeonMake(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL,
                                           322, 42, 210, 250, IDC_DG_PROFILE);
        DungeonMake(L"BUTTON", L"NHÂN BẢN", BS_PUSHBUTTON, 540, 42, 90, 27, IDC_DG_DUPLICATE);
        DungeonMake(L"BUTTON", L"XÓA", BS_PUSHBUTTON, 638, 42, 60, 27, IDC_DG_DELETE_PROFILE);
        DungeonMake(L"BUTTON", L"CHẠY PHÓ BẢN", BS_DEFPUSHBUTTON, 708, 42, 155, 27, IDC_DG_START);
        DungeonMake(L"BUTTON", L"DỪNG", BS_PUSHBUTTON, 871, 42, 82, 27, IDC_DG_STOP);

        DungeonMake(L"STATIC", L"Tên:", SS_LEFT | SS_CENTERIMAGE, 18, 76, 35, 27, 0);
        dungeonProfileName_ = DungeonMake(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL,
                                          55, 76, 208, 27, IDC_DG_PROFILE_NAME);
        dungeonTemplateCombo_ = DungeonMake(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL,
                                            272, 76, 230, 330, IDC_DG_TEMPLATE);
        for (const DungeonPreset& preset : kDungeonPresets)
            SendMessageW(dungeonTemplateCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(preset.name));
        SendMessageW(dungeonTemplateCombo_, CB_SETCURSEL, 0, 0);
        DungeonMake(L"BUTTON", L"+ THÊM MẪU", BS_PUSHBUTTON, 510, 76, 110, 27, IDC_DG_ADD_TEMPLATE);
        dungeonEntrySequenceButton_ = DungeonMake(L"BUTTON", L"CHUỖI VÀO", BS_PUSHBUTTON,
                                                  628, 76, 105, 27, IDC_DG_ENTRY_SEQUENCE);
        dungeonExitSequenceButton_ = DungeonMake(L"BUTTON", L"CHUỖI RA", BS_PUSHBUTTON,
                                                 741, 76, 100, 27, IDC_DG_EXIT_SEQUENCE);
        DungeonMake(L"BUTTON", L"LƯU HỒ SƠ", BS_PUSHBUTTON, 849, 76, 104, 27, IDC_DG_SAVE_PROFILE);

        dungeonStatus_ = DungeonMake(L"STATIC", L"AUTO PHÓ BẢN: đã dừng",
                                     SS_LEFT | SS_CENTERIMAGE | WS_BORDER, 18, 109, 935, 32, IDC_DG_STATUS);

        DungeonMake(L"STATIC", L"NPC ResID:", SS_LEFT | SS_CENTERIMAGE, 18, 148, 72, 25, 0);
        dungeonNpcResID_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 92, 148, 62, 25, IDC_DG_NPC_RESID);
        DungeonMake(L"STATIC", L"Map NPC:", SS_LEFT | SS_CENTERIMAGE, 164, 148, 60, 25, 0);
        dungeonGatherMap_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 225, 148, 52, 25, IDC_DG_GATHER_MAP);
        DungeonMake(L"STATIC", L"X:", SS_LEFT | SS_CENTERIMAGE, 287, 148, 18, 25, 0);
        dungeonNpcX_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 306, 148, 70, 25, IDC_DG_NPC_X);
        DungeonMake(L"STATIC", L"Y:", SS_LEFT | SS_CENTERIMAGE, 384, 148, 18, 25, 0);
        dungeonNpcY_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 403, 148, 70, 25, IDC_DG_NPC_Y);
        DungeonMake(L"BUTTON", L"GET TỌA NPC", BS_PUSHBUTTON, 481, 148, 110, 25, IDC_DG_GET_NPC_POS);
        DungeonMake(L"STATIC", L"Map phó bản:", SS_LEFT | SS_CENTERIMAGE, 603, 148, 85, 25, 0);
        dungeonMap_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 690, 148, 55, 25, IDC_DG_MAP);
        dungeonLoop_ = DungeonMake(L"BUTTON", L"Lặp vô hạn", BS_AUTOCHECKBOX, 760, 148, 100, 25, IDC_DG_LOOP);
        Button_SetCheck(dungeonLoop_, BST_CHECKED);
        DungeonMake(L"STATIC", L"ClickNPC dùng ResID; dialog vào/ra dùng chuỗi REAL CLICK có thể tự sửa.",
                    SS_LEFT | SS_CENTERIMAGE, 18, 176, 935, 20, 0);

        dungeonStageList_ = DungeonMake(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER,
                                        18, 199, 935, 160, IDC_DG_STAGE_LIST);
        ListView_SetExtendedListViewStyle(dungeonStageList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        DungeonListColumn(dungeonStageList_, 0, 30, L"#");
        DungeonListColumn(dungeonStageList_, 1, 145, L"Bước/tọa");
        DungeonListColumn(dungeonStageList_, 2, 45, L"Map");
        DungeonListColumn(dungeonStageList_, 3, 105, L"X,Y");
        DungeonListColumn(dungeonStageList_, 4, 55, L"Cần");
        DungeonListColumn(dungeonStageList_, 5, 90, L"Bộ lọc");
        DungeonListColumn(dungeonStageList_, 6, 60, L"BOSS");
        DungeonListColumn(dungeonStageList_, 7, 70, L"Timeout");
        DungeonListColumn(dungeonStageList_, 8, 300, L"Tiến độ runtime");

        DungeonMake(L"BUTTON", L"+ BƯỚC", BS_PUSHBUTTON, 18, 365, 80, 25, IDC_DG_STAGE_ADD);
        DungeonMake(L"BUTTON", L"- XÓA", BS_PUSHBUTTON, 103, 365, 70, 25, IDC_DG_STAGE_DELETE);
        DungeonMake(L"BUTTON", L"LÊN", BS_PUSHBUTTON, 178, 365, 55, 25, IDC_DG_STAGE_UP);
        DungeonMake(L"BUTTON", L"XUỐNG", BS_PUSHBUTTON, 238, 365, 60, 25, IDC_DG_STAGE_DOWN);
        dungeonStageName_ = DungeonMake(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 306, 365, 115, 25, IDC_DG_STAGE_NAME);
        dungeonStageMap_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 426, 365, 45, 25, IDC_DG_STAGE_MAP);
        dungeonStageX_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 476, 365, 60, 25, IDC_DG_STAGE_X);
        dungeonStageY_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 541, 365, 60, 25, IDC_DG_STAGE_Y);
        dungeonStageKills_ = DungeonMake(L"EDIT", L"40", WS_BORDER | ES_NUMBER | ES_CENTER, 606, 365, 45, 25, IDC_DG_STAGE_KILLS);
        dungeonStageGroup_ = DungeonMake(L"EDIT", L"THUONG", WS_BORDER | ES_AUTOHSCROLL, 656, 365, 80, 25, IDC_DG_STAGE_GROUP);
        dungeonStageBoss_ = DungeonMake(L"BUTTON", L"Boss", BS_AUTOCHECKBOX, 741, 365, 55, 25, IDC_DG_STAGE_BOSS);
        DungeonMake(L"BUTTON", L"GET TỌA", BS_PUSHBUTTON, 801, 365, 72, 25, IDC_DG_STAGE_GET_POS);
        DungeonMake(L"BUTTON", L"LƯU", BS_PUSHBUTTON, 879, 365, 74, 25, IDC_DG_STAGE_SAVE);

        DungeonMake(L"STATIC", L"Sai số:", SS_LEFT | SS_CENTERIMAGE, 306, 394, 48, 23, 0);
        dungeonStageTolerance_ = DungeonMake(L"EDIT", L"120", WS_BORDER | ES_NUMBER | ES_CENTER, 355, 394, 50, 23, IDC_DG_STAGE_TOLERANCE);
        DungeonMake(L"STATIC", L"Bán kính đếm (0=AOI):", SS_LEFT | SS_CENTERIMAGE, 415, 394, 130, 23, 0);
        dungeonStageRadius_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 547, 394, 58, 23, IDC_DG_STAGE_RADIUS);
        DungeonMake(L"STATIC", L"Timeout giây:", SS_LEFT | SS_CENTERIMAGE, 615, 394, 82, 23, 0);
        dungeonStageTimeout_ = DungeonMake(L"EDIT", L"900", WS_BORDER | ES_NUMBER | ES_CENTER, 699, 394, 58, 23, IDC_DG_STAGE_TIMEOUT);
        DungeonMake(L"STATIC", L"Mỗi bước chỉ PASS khi đủ death event; timeout luôn là LỖI, không phải thành công.",
                    SS_LEFT | SS_CENTERIMAGE, 18, 421, 935, 20, 0);

        dungeonScanList_ = DungeonMake(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER,
                                       18, 445, 935, 145, IDC_DG_SCAN_LIST);
        ListView_SetExtendedListViewStyle(dungeonScanList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        DungeonListColumn(dungeonScanList_, 0, 185, L"Tên");
        DungeonListColumn(dungeonScanList_, 1, 85, L"RoleID động");
        DungeonListColumn(dungeonScanList_, 2, 75, L"ResID");
        DungeonListColumn(dungeonScanList_, 3, 120, L"HP / MaxHP");
        DungeonListColumn(dungeonScanList_, 4, 55, L"Chết");
        DungeonListColumn(dungeonScanList_, 5, 90, L"X,Y");
        DungeonListColumn(dungeonScanList_, 6, 165, L"Class / Type");
        DungeonListColumn(dungeonScanList_, 7, 150, L"Cờ dữ liệu");
        DungeonMake(L"BUTTON", L"QUÉT QUÁI / HP", BS_PUSHBUTTON, 18, 596, 140, 27, IDC_DG_SCAN_NOW);
        DungeonMake(L"BUTTON", L"THÊM QUÁI ĐÃ CHỌN VÀO BỘ ĐẾM", BS_PUSHBUTTON, 166, 596, 280, 27, IDC_DG_SCAN_ADD_RULE);
        DungeonMake(L"STATIC", L"Không cộng khi quái chỉ biến khỏi AOI; cần bắt được chuyển sống → HP=0/chết.",
                    SS_LEFT | SS_CENTERIMAGE, 460, 596, 493, 27, 0);

        dungeonRuleList_ = DungeonMake(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER,
                                       18, 630, 935, 130, IDC_DG_RULE_LIST);
        ListView_SetExtendedListViewStyle(dungeonRuleList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
                                                               LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER);
        DungeonListColumn(dungeonRuleList_, 0, 230, L"Tên quái lưu");
        DungeonListColumn(dungeonRuleList_, 1, 100, L"ResID ổn định");
        DungeonListColumn(dungeonRuleList_, 2, 160, L"Nhóm lọc");
        DungeonListColumn(dungeonRuleList_, 3, 80, L"BOSS");
        DungeonListColumn(dungeonRuleList_, 4, 365, L"Cách đếm");

        DungeonMake(L"STATIC", L"Tên:", SS_LEFT | SS_CENTERIMAGE, 18, 768, 35, 25, 0);
        dungeonRuleName_ = DungeonMake(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 55, 768, 190, 25, IDC_DG_RULE_NAME);
        DungeonMake(L"STATIC", L"ResID:", SS_LEFT | SS_CENTERIMAGE, 254, 768, 42, 25, 0);
        dungeonRuleResID_ = DungeonMake(L"EDIT", L"0", WS_BORDER | ES_NUMBER | ES_CENTER, 298, 768, 68, 25, IDC_DG_RULE_RESID);
        DungeonMake(L"STATIC", L"Nhóm:", SS_LEFT | SS_CENTERIMAGE, 376, 768, 42, 25, 0);
        dungeonRuleGroup_ = DungeonMake(L"EDIT", L"THUONG", WS_BORDER | ES_AUTOHSCROLL, 420, 768, 100, 25, IDC_DG_RULE_GROUP);
        dungeonRuleBoss_ = DungeonMake(L"BUTTON", L"BOSS", BS_AUTOCHECKBOX, 530, 768, 65, 25, IDC_DG_RULE_BOSS);
        DungeonMake(L"BUTTON", L"LƯU BỘ LỌC", BS_PUSHBUTTON, 605, 768, 120, 25, IDC_DG_RULE_SAVE);
        DungeonMake(L"BUTTON", L"XÓA BỘ LỌC", BS_PUSHBUTTON, 733, 768, 120, 25, IDC_DG_RULE_DELETE);
        DungeonMake(L"STATIC", L"ResID>0 ưu tiên khớp ID; nếu client chưa trả ResID thì khớp tên chính xác.",
                    SS_LEFT | SS_CENTERIMAGE, 18, 800, 935, 25, 0);
        DungeonMake(L"STATIC", L"TRẠNG THÁI KIỂM THỬ: Source/CI có thể PASS; scanner HP/AutoFight/FSM vẫn phải TEST TRỰC TIẾP đúng client game.",
                    SS_LEFT | SS_CENTERIMAGE | WS_BORDER, 18, 835, 935, 35, 0);
    }

    DungeonProfile* CurrentDungeonProfile() {
        if (dungeonProfileIndex_ < 0 || dungeonProfileIndex_ >= static_cast<int>(dungeonProfiles_.size())) return nullptr;
        return &dungeonProfiles_[static_cast<std::size_t>(dungeonProfileIndex_)];
    }

    Account* SelectedDungeonAccount() {
        if (!dungeonAccountCombo_) return nullptr;
        const LRESULT selected = SendMessageW(dungeonAccountCombo_, CB_GETCURSEL, 0, 0);
        if (selected == CB_ERR) return nullptr;
        const LRESULT data = SendMessageW(dungeonAccountCombo_, CB_GETITEMDATA, selected, 0);
        return data == CB_ERR ? nullptr : AccountByPid(static_cast<DWORD>(data));
    }

    void RefreshDungeonAccountCombo() {
        if (!dungeonAccountCombo_) return;
        DWORD keepPid = 0;
        if (Account* selected = SelectedDungeonAccount()) keepPid = selected->game.pid;
        SendMessageW(dungeonAccountCombo_, CB_RESETCONTENT, 0, 0);
        int selectIndex = 0;
        for (std::size_t i = 0; i < accounts_.size(); ++i) {
            const Account& account = *accounts_[i];
            const LRESULT row = SendMessageW(dungeonAccountCombo_, CB_ADDSTRING, 0,
                                              reinterpret_cast<LPARAM>(AccountTag(account).c_str()));
            SendMessageW(dungeonAccountCombo_, CB_SETITEMDATA, row, static_cast<LPARAM>(account.game.pid));
            if (account.game.pid == keepPid || (dungeonRuntime_.running && account.game.pid == dungeonRuntime_.pid))
                selectIndex = static_cast<int>(row);
        }
        if (!accounts_.empty()) SendMessageW(dungeonAccountCombo_, CB_SETCURSEL, selectIndex, 0);
    }

    void RefreshDungeonProfileCombo() {
        if (!dungeonProfileCombo_) return;
        SendMessageW(dungeonProfileCombo_, CB_RESETCONTENT, 0, 0);
        for (const DungeonProfile& profile : dungeonProfiles_)
            SendMessageW(dungeonProfileCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(profile.name.c_str()));
        if (!dungeonProfiles_.empty()) {
            dungeonProfileIndex_ = std::clamp(dungeonProfileIndex_, 0, static_cast<int>(dungeonProfiles_.size()) - 1);
            SendMessageW(dungeonProfileCombo_, CB_SETCURSEL, dungeonProfileIndex_, 0);
        }
    }

    void RefreshDungeonStageList() {
        if (!dungeonStageList_) return;
        const int keep = ListView_GetNextItem(dungeonStageList_, -1, LVNI_SELECTED);
        ListView_DeleteAllItems(dungeonStageList_);
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile) return;
        for (std::size_t i = 0; i < profile->stages.size(); ++i) {
            const DungeonStage& stage = profile->stages[i];
            const std::wstring index = std::to_wstring(i + 1);
            LVITEMW item{}; item.mask = LVIF_TEXT; item.iItem = static_cast<int>(i);
            item.pszText = const_cast<wchar_t*>(index.c_str()); ListView_InsertItem(dungeonStageList_, &item);
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 1, const_cast<wchar_t*>(stage.name.c_str()));
            const std::wstring map = std::to_wstring(stage.mapID);
            const std::wstring pos = std::to_wstring(stage.x) + L"," + std::to_wstring(stage.y);
            const std::wstring kills = std::to_wstring(stage.requiredKills);
            const std::wstring boss = stage.boss ? L"CÓ" : L"-";
            const std::wstring timeout = std::to_wstring(stage.timeoutSec) + L"s";
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 2, const_cast<wchar_t*>(map.c_str()));
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 3, const_cast<wchar_t*>(pos.c_str()));
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 4, const_cast<wchar_t*>(kills.c_str()));
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 5, const_cast<wchar_t*>(stage.filterGroup.c_str()));
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 6, const_cast<wchar_t*>(boss.c_str()));
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 7, const_cast<wchar_t*>(timeout.c_str()));
            std::wstring progress = L"-";
            if (dungeonRuntime_.profileIndex == dungeonProfileIndex_ &&
                i < dungeonRuntime_.stageKillCounts.size()) {
                const int value = dungeonRuntime_.stageKillCounts[i];
                progress = std::to_wstring(value) + L" / " + kills;
                if (value >= stage.requiredKills) progress = L"ĐỦ • " + progress;
                else if (dungeonRuntime_.running && dungeonRuntime_.stageIndex == static_cast<int>(i))
                    progress = L"ĐANG ĐẾM • " + progress;
            }
            ListView_SetItemText(dungeonStageList_, static_cast<int>(i), 8, const_cast<wchar_t*>(progress.c_str()));
        }
        if (!profile->stages.empty()) {
            const int selected = std::clamp(keep < 0 ? 0 : keep, 0, static_cast<int>(profile->stages.size()) - 1);
            ListView_SetItemState(dungeonStageList_, selected, LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
        }
    }

    void LoadDungeonStageEditor(int row = -1) {
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile || profile->stages.empty()) return;
        if (row < 0) row = ListView_GetNextItem(dungeonStageList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(profile->stages.size())) return;
        const DungeonStage& stage = profile->stages[static_cast<std::size_t>(row)];
        SetText(dungeonStageName_, stage.name);
        SetText(dungeonStageMap_, std::to_wstring(stage.mapID));
        SetText(dungeonStageX_, std::to_wstring(stage.x));
        SetText(dungeonStageY_, std::to_wstring(stage.y));
        SetText(dungeonStageKills_, std::to_wstring(stage.requiredKills));
        SetText(dungeonStageGroup_, stage.filterGroup);
        SetText(dungeonStageTolerance_, std::to_wstring(stage.tolerance));
        SetText(dungeonStageRadius_, std::to_wstring(stage.countRadius));
        SetText(dungeonStageTimeout_, std::to_wstring(stage.timeoutSec));
        Button_SetCheck(dungeonStageBoss_, stage.boss ? BST_CHECKED : BST_UNCHECKED);
    }

    void RefreshDungeonRuleList() {
        if (!dungeonRuleList_) return;
        dungeonRuleUiLoading_ = true;
        const int keep = ListView_GetNextItem(dungeonRuleList_, -1, LVNI_SELECTED);
        ListView_DeleteAllItems(dungeonRuleList_);
        DungeonProfile* profile = CurrentDungeonProfile();
        if (profile) {
            for (std::size_t i = 0; i < profile->rules.size(); ++i) {
                const MonsterRule& rule = profile->rules[i];
                LVITEMW item{}; item.mask = LVIF_TEXT; item.iItem = static_cast<int>(i);
                item.pszText = const_cast<wchar_t*>(rule.name.c_str()); ListView_InsertItem(dungeonRuleList_, &item);
                const std::wstring res = std::to_wstring(rule.resID);
                const std::wstring boss = rule.boss ? L"CÓ" : L"-";
                const std::wstring mode = rule.resID > 0
                    ? L"ResID+RoleID life: sống → HP=0"
                    : L"Tên+RoleID life: sống → HP=0";
                ListView_SetItemText(dungeonRuleList_, static_cast<int>(i), 1, const_cast<wchar_t*>(res.c_str()));
                ListView_SetItemText(dungeonRuleList_, static_cast<int>(i), 2, const_cast<wchar_t*>(rule.group.c_str()));
                ListView_SetItemText(dungeonRuleList_, static_cast<int>(i), 3, const_cast<wchar_t*>(boss.c_str()));
                ListView_SetItemText(dungeonRuleList_, static_cast<int>(i), 4, const_cast<wchar_t*>(mode.c_str()));
                ListView_SetCheckState(dungeonRuleList_, static_cast<int>(i), rule.enabled ? TRUE : FALSE);
            }
            if (!profile->rules.empty()) {
                const int selected = std::clamp(keep < 0 ? 0 : keep, 0, static_cast<int>(profile->rules.size()) - 1);
                ListView_SetItemState(dungeonRuleList_, selected, LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED);
            }
        }
        dungeonRuleUiLoading_ = false;
    }

    void LoadDungeonRuleEditor(int row = -1) {
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile || profile->rules.empty()) return;
        if (row < 0) row = ListView_GetNextItem(dungeonRuleList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(profile->rules.size())) return;
        const MonsterRule& rule = profile->rules[static_cast<std::size_t>(row)];
        SetText(dungeonRuleName_, rule.name);
        SetText(dungeonRuleResID_, std::to_wstring(rule.resID));
        SetText(dungeonRuleGroup_, rule.group);
        Button_SetCheck(dungeonRuleBoss_, rule.boss ? BST_CHECKED : BST_UNCHECKED);
    }

    void LoadDungeonProfileToUi() {
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile) return;
        SetText(dungeonProfileName_, profile->name);
        SetText(dungeonNpcResID_, std::to_wstring(profile->npcResID));
        SetText(dungeonGatherMap_, std::to_wstring(profile->gatherMapID));
        SetText(dungeonNpcX_, std::to_wstring(profile->npcX));
        SetText(dungeonNpcY_, std::to_wstring(profile->npcY));
        SetText(dungeonMap_, std::to_wstring(profile->dungeonMapID));
        Button_SetCheck(dungeonLoop_, profile->loop ? BST_CHECKED : BST_UNCHECKED);
        SetText(dungeonEntrySequenceButton_, L"CHUỖI VÀO (" + std::to_wstring(profile->entryClicks.size()) + L")");
        SetText(dungeonExitSequenceButton_, L"CHUỖI RA (" + std::to_wstring(profile->exitClicks.size()) + L")");
        RefreshDungeonStageList();
        LoadDungeonStageEditor();
        RefreshDungeonRuleList();
        LoadDungeonRuleEditor();
    }

    void RefreshDungeonPage() {
        RefreshDungeonAccountCombo();
        RefreshDungeonProfileCombo();
        LoadDungeonProfileToUi();
        if (dungeonStatus_) SetText(dungeonStatus_, L"AUTO PHÓ BẢN: " + dungeonRuntime_.status);
    }

    void SaveDungeonProfileHeader() {
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile) return;
        std::wstring name = SanitizeSpotName(GetText(dungeonProfileName_));
        if (name.empty()) name = L"PHÓ BẢN " + std::to_wstring(dungeonProfileIndex_ + 1);
        profile->name = name;
        profile->npcResID = std::max(0, _wtoi(GetText(dungeonNpcResID_).c_str()));
        profile->gatherMapID = std::max(0, _wtoi(GetText(dungeonGatherMap_).c_str()));
        profile->npcX = _wtoi(GetText(dungeonNpcX_).c_str());
        profile->npcY = _wtoi(GetText(dungeonNpcY_).c_str());
        profile->dungeonMapID = std::max(0, _wtoi(GetText(dungeonMap_).c_str()));
        profile->loop = Button_GetCheck(dungeonLoop_) == BST_CHECKED;
        SaveDungeonProfiles();
        RefreshDungeonProfileCombo();
        Log(L"[AUTO PHÓ BẢN] Đã lưu hồ sơ " + profile->name + L".");
    }

    void AddDungeonTemplate() {
        int index = static_cast<int>(SendMessageW(dungeonTemplateCombo_, CB_GETCURSEL, 0, 0));
        if (index < 0 || index >= static_cast<int>(kDungeonPresets.size())) index = 0;
        const DungeonPreset& preset = kDungeonPresets[static_cast<std::size_t>(index)];
        dungeonProfiles_.push_back(MakeDungeonProfile(preset.name, preset.dungeonMapID,
                                                       preset.gatherMapID, preset.npcResID,
                                                       preset.npcX, preset.npcY));
        dungeonProfileIndex_ = static_cast<int>(dungeonProfiles_.size()) - 1;
        SaveDungeonProfiles();
        RefreshDungeonProfileCombo();
        LoadDungeonProfileToUi();
        Log(L"[AUTO PHÓ BẢN] Đã thêm mẫu " + std::wstring(preset.name) +
            L"; tọa các bãi vẫn để 0 để người dùng GET trực tiếp trong map.");
    }

    void DuplicateDungeonProfile() {
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile) return;
        DungeonProfile copy = *profile;
        copy.name += L" (BẢN SAO)";
        dungeonProfiles_.push_back(std::move(copy));
        dungeonProfileIndex_ = static_cast<int>(dungeonProfiles_.size()) - 1;
        SaveDungeonProfiles(); RefreshDungeonProfileCombo(); LoadDungeonProfileToUi();
    }

    void DeleteDungeonProfile() {
        if (dungeonProfiles_.empty()) return;
        if (dungeonRuntime_.running) {
            MessageBoxW(hwnd_, L"Hãy dừng AUTO PHÓ BẢN trước khi xóa bất kỳ hồ sơ nào để không làm lệch profile index runtime.", kTitle, MB_OK | MB_ICONWARNING);
            return;
        }
        if (MessageBoxW(hwnd_, L"Xóa hồ sơ phó bản đang chọn?", kTitle,
                        MB_YESNO | MB_ICONQUESTION) != IDYES) return;
        dungeonProfiles_.erase(dungeonProfiles_.begin() + dungeonProfileIndex_);
        dungeonRuntime_.profileIndex = -1;
        dungeonRuntime_.stageKillCounts.clear();
        if (dungeonProfiles_.empty()) dungeonProfiles_.push_back(MakeDungeonProfile(L"PHÓ BẢN MẪU"));
        dungeonProfileIndex_ = std::clamp(dungeonProfileIndex_, 0, static_cast<int>(dungeonProfiles_.size()) - 1);
        SaveDungeonProfiles(); RefreshDungeonProfileCombo(); LoadDungeonProfileToUi();
    }

    void SelectDungeonProfile() {
        const int selected = static_cast<int>(SendMessageW(dungeonProfileCombo_, CB_GETCURSEL, 0, 0));
        if (selected < 0 || selected >= static_cast<int>(dungeonProfiles_.size())) return;
        dungeonProfileIndex_ = selected;
        LoadDungeonProfileToUi();
    }

    void AddDungeonStage() {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        DungeonStage stage{};
        stage.name = L"TỌA " + std::to_wstring(profile->stages.size() + 1);
        stage.mapID = profile->dungeonMapID;
        profile->stages.push_back(std::move(stage));
        SaveDungeonProfiles(); RefreshDungeonStageList();
        const int row = static_cast<int>(profile->stages.size()) - 1;
        ListView_SetItemState(dungeonStageList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonStageEditor(row);
    }

    void DeleteDungeonStage() {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        const int row = ListView_GetNextItem(dungeonStageList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(profile->stages.size())) return;
        profile->stages.erase(profile->stages.begin() + row);
        SaveDungeonProfiles(); RefreshDungeonStageList(); LoadDungeonStageEditor();
    }

    void MoveDungeonStage(int direction) {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        const int row = ListView_GetNextItem(dungeonStageList_, -1, LVNI_SELECTED);
        const int target = row + direction;
        if (row < 0 || target < 0 || row >= static_cast<int>(profile->stages.size()) ||
            target >= static_cast<int>(profile->stages.size())) return;
        std::swap(profile->stages[static_cast<std::size_t>(row)], profile->stages[static_cast<std::size_t>(target)]);
        SaveDungeonProfiles(); RefreshDungeonStageList();
        ListView_SetItemState(dungeonStageList_, target, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonStageEditor(target);
    }

    void SaveDungeonStageEditor() {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        const int row = ListView_GetNextItem(dungeonStageList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(profile->stages.size())) return;
        DungeonStage& stage = profile->stages[static_cast<std::size_t>(row)];
        stage.name = SanitizeSpotName(GetText(dungeonStageName_));
        if (stage.name.empty()) stage.name = L"TỌA " + std::to_wstring(row + 1);
        stage.mapID = std::max(0, _wtoi(GetText(dungeonStageMap_).c_str()));
        stage.x = _wtoi(GetText(dungeonStageX_).c_str());
        stage.y = _wtoi(GetText(dungeonStageY_).c_str());
        stage.requiredKills = std::clamp(_wtoi(GetText(dungeonStageKills_).c_str()), 1, 100000);
        stage.filterGroup = SanitizeSpotName(GetText(dungeonStageGroup_));
        if (stage.filterGroup.empty()) stage.filterGroup = L"THUONG";
        stage.tolerance = std::clamp(_wtoi(GetText(dungeonStageTolerance_).c_str()), 20, 5000);
        stage.countRadius = std::clamp(_wtoi(GetText(dungeonStageRadius_).c_str()), 0, 100000);
        stage.timeoutSec = std::clamp(_wtoi(GetText(dungeonStageTimeout_).c_str()), 10, 86400);
        stage.boss = Button_GetCheck(dungeonStageBoss_) == BST_CHECKED;
        SaveDungeonProfiles(); RefreshDungeonStageList();
        ListView_SetItemState(dungeonStageList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonStageEditor(row);
    }

    void CaptureDungeonPosition(bool npc) {
        Account* account = SelectedDungeonAccount();
        if (!account) { Log(L"[AUTO PHÓ BẢN] Chưa chọn acc để GET tọa."); return; }
        std::wstring error;
        if (!ReadSnapshot(*account, error, 1200) ||
            (account->snapshot.validMask & (ValidMap | ValidPosition)) != (ValidMap | ValidPosition) ||
            !account->snapshot.mapReady || account->snapshot.waitingChangeMap) {
            LogAccount(*account, L"GET tọa phó bản FAIL: " + error); return;
        }
        if (npc) {
            SetText(dungeonGatherMap_, std::to_wstring(account->snapshot.mapID));
            SetText(dungeonNpcX_, std::to_wstring(account->snapshot.x));
            SetText(dungeonNpcY_, std::to_wstring(account->snapshot.y));
            SaveDungeonProfileHeader();
        } else {
            SetText(dungeonStageMap_, std::to_wstring(account->snapshot.mapID));
            SetText(dungeonStageX_, std::to_wstring(account->snapshot.x));
            SetText(dungeonStageY_, std::to_wstring(account->snapshot.y));
            SaveDungeonStageEditor();
        }
    }

    void RefreshDungeonScanList(const Response& response) {
        dungeonLastScan_.assign(response.monsters, response.monsters + response.monsterCount);
        if (!dungeonScanList_) return;
        ListView_DeleteAllItems(dungeonScanList_);
        for (std::size_t i = 0; i < dungeonLastScan_.size(); ++i) {
            const MonsterRecord& monster = dungeonLastScan_[i];
            const std::wstring name = monster.name[0] ? monster.name : L"?";
            LVITEMW item{}; item.mask = LVIF_TEXT; item.iItem = static_cast<int>(i);
            item.pszText = const_cast<wchar_t*>(name.c_str()); ListView_InsertItem(dungeonScanList_, &item);
            const std::wstring role = std::to_wstring(monster.roleID);
            const std::wstring res = (monster.validMask & MonsterValidTemplate) ? std::to_wstring(monster.resID) : L"?";
            const std::wstring hp = std::to_wstring(monster.hp) + L" / " + std::to_wstring(monster.maxHP);
            const std::wstring dead = monster.dead ? L"CÓ" : L"-";
            const std::wstring pos = (monster.validMask & MonsterValidPosition)
                ? std::to_wstring(monster.x) + L"," + std::to_wstring(monster.y) : L"?";
            const std::wstring klass = std::wstring(monster.className) + L" / " + std::to_wstring(monster.type);
            const std::wstring flags = L"0x" + std::to_wstring(monster.validMask);
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 1, const_cast<wchar_t*>(role.c_str()));
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 2, const_cast<wchar_t*>(res.c_str()));
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 3, const_cast<wchar_t*>(hp.c_str()));
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 4, const_cast<wchar_t*>(dead.c_str()));
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 5, const_cast<wchar_t*>(pos.c_str()));
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 6, const_cast<wchar_t*>(klass.c_str()));
            ListView_SetItemText(dungeonScanList_, static_cast<int>(i), 7, const_cast<wchar_t*>(flags.c_str()));
        }
        if (!dungeonLastScan_.empty())
            ListView_SetItemState(dungeonScanList_, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    bool ScanDungeonMonsters(Account& account, Response& response, std::wstring& error) {
        if (!EnsureAttach(account, error)) return false;
        return account.bridge.Call(Command::ScanNearbyMonsters, 0, 0, 0, response, error, 1800);
    }

    void ManualDungeonScan() {
        Account* account = SelectedDungeonAccount();
        if (!account) { Log(L"[AUTO PHÓ BẢN] Chưa chọn acc để quét."); return; }
        Response response{}; std::wstring error;
        if (!ScanDungeonMonsters(*account, response, error)) {
            SetText(dungeonStatus_, L"QUÉT FAIL: " + error);
            LogAccount(*account, L"QUÉT QUÁI/HP FAIL: " + error); return;
        }
        RefreshDungeonScanList(response);
        SetText(dungeonStatus_, L"QUÉT PASS: " + std::to_wstring(response.monsterCount) +
                                 L" role có HP • xem Class/Type rồi thêm đúng quái vào bộ lọc");
    }

    void AddSelectedScanToRule() {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        const int row = ListView_GetNextItem(dungeonScanList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(dungeonLastScan_.size())) return;
        const MonsterRecord& monster = dungeonLastScan_[static_cast<std::size_t>(row)];
        MonsterRule rule{};
        rule.name = monster.name;
        rule.resID = (monster.validMask & MonsterValidTemplate) ? monster.resID : 0;
        rule.group = L"THUONG";
        rule.enabled = true;
        rule.boss = false;
        const auto duplicate = std::find_if(profile->rules.begin(), profile->rules.end(), [&](const MonsterRule& item) {
            return (rule.resID > 0 && item.resID == rule.resID) ||
                   (rule.resID == 0 && EqualFolded(item.name, rule.name));
        });
        if (duplicate == profile->rules.end()) profile->rules.push_back(std::move(rule));
        SaveDungeonProfiles(); RefreshDungeonRuleList(); LoadDungeonRuleEditor();
    }

    void SaveDungeonRuleEditor() {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        int row = ListView_GetNextItem(dungeonRuleList_, -1, LVNI_SELECTED);
        MonsterRule rule{};
        rule.name = SanitizeSpotName(GetText(dungeonRuleName_));
        rule.resID = std::max(0, _wtoi(GetText(dungeonRuleResID_).c_str()));
        rule.group = SanitizeSpotName(GetText(dungeonRuleGroup_));
        if (rule.group.empty()) rule.group = L"THUONG";
        rule.boss = Button_GetCheck(dungeonRuleBoss_) == BST_CHECKED;
        rule.enabled = row >= 0 && row < static_cast<int>(profile->rules.size())
            ? profile->rules[static_cast<std::size_t>(row)].enabled : true;
        if (rule.name.empty() && rule.resID <= 0) {
            Log(L"[AUTO PHÓ BẢN] Bộ lọc cần ResID hoặc tên."); return;
        }
        if (row >= 0 && row < static_cast<int>(profile->rules.size()))
            profile->rules[static_cast<std::size_t>(row)] = std::move(rule);
        else {
            profile->rules.push_back(std::move(rule));
            row = static_cast<int>(profile->rules.size()) - 1;
        }
        SaveDungeonProfiles(); RefreshDungeonRuleList();
        ListView_SetItemState(dungeonRuleList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonRuleEditor(row);
    }

    void DeleteDungeonRule() {
        DungeonProfile* profile = CurrentDungeonProfile(); if (!profile) return;
        const int row = ListView_GetNextItem(dungeonRuleList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(profile->rules.size())) return;
        profile->rules.erase(profile->rules.begin() + row);
        SaveDungeonProfiles(); RefreshDungeonRuleList(); LoadDungeonRuleEditor();
    }

    void PersistDungeonRuleChecks(const NMLISTVIEW& change) {
        if (dungeonRuleUiLoading_) return;
        if ((change.uChanged & LVIF_STATE) == 0 ||
            ((change.uOldState ^ change.uNewState) & LVIS_STATEIMAGEMASK) == 0) return;
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile || change.iItem < 0 || change.iItem >= static_cast<int>(profile->rules.size())) return;
        profile->rules[static_cast<std::size_t>(change.iItem)].enabled =
            ListView_GetCheckState(dungeonRuleList_, change.iItem) != FALSE;
        SaveDungeonProfiles();
    }

    bool ValidateDungeonProfile(const DungeonProfile& profile, std::wstring& error) const {
        if (profile.name.empty()) { error = L"Tên hồ sơ trống"; return false; }
        if (profile.gatherMapID <= 0 || profile.npcResID <= 0) {
            error = L"Cần Map NPC và NPC ResID > 0"; return false;
        }
        if (profile.npcX == 0 && profile.npcY == 0) {
            error = L"Chưa GET/nhập tọa NPC"; return false;
        }
        if (profile.dungeonMapID <= 0) { error = L"Map phó bản phải > 0"; return false; }
        if (profile.stages.empty()) { error = L"Phó bản chưa có bước/tọa đánh"; return false; }
        for (std::size_t i = 0; i < profile.stages.size(); ++i) {
            const DungeonStage& stage = profile.stages[i];
            if (stage.mapID <= 0 || (stage.x == 0 && stage.y == 0) || stage.requiredKills <= 0) {
                error = L"Bước " + std::to_wstring(i + 1) + L" thiếu Map/X/Y hoặc số quái cần giết";
                return false;
            }
            const bool hasRule = std::any_of(profile.rules.begin(), profile.rules.end(), [&](const MonsterRule& rule) {
                return rule.enabled && EqualFolded(rule.group, stage.filterGroup) &&
                       (rule.resID > 0 || !rule.name.empty());
            });
            if (!hasRule) {
                error = L"Bước " + std::to_wstring(i + 1) + L" chưa có bộ lọc quái BẬT thuộc nhóm " + stage.filterGroup;
                return false;
            }
        }
        return true;
    }

    void SetDungeonRuntimeStatus(const std::wstring& text) {
        dungeonRuntime_.status = text;
        if (dungeonStatus_) SetText(dungeonStatus_, L"AUTO PHÓ BẢN: " + text);
        Account* account = AccountByPid(dungeonRuntime_.pid);
        if (account && dungeonRuntime_.running) account->runtime.status = L"PHÓ BẢN • " + text;
    }

    void BeginDungeonPhase(DungeonPhase phase, const std::wstring& status, DWORD now) {
        dungeonRuntime_.phase = phase;
        dungeonRuntime_.phaseTick = now;
        dungeonRuntime_.lastCommandTick = 0;
        dungeonRuntime_.commandAttempts = 0;
        dungeonRuntime_.clickIndex = 0;
        dungeonRuntime_.clickRepeatDone = 0;
        SetDungeonRuntimeStatus(status);
    }

    void ReleaseDungeonClickLease(const std::wstring& reason) {
        if (dungeonRuntime_.clickLeaseHeld) {
            ReleaseCoordinatorSequenceLease(reason);
            dungeonRuntime_.clickLeaseHeld = false;
        }
    }

    void StopDungeonInternal(const std::wstring& reason, bool failure) {
        Account* account = AccountByPid(dungeonRuntime_.pid);
        ReleaseDungeonClickLease(L"AUTO PHÓ BẢN kết thúc/dừng");
        if (account && account->bridge.Attached() && !account->runtime.clientFreezeActive) {
            Response response{}; std::wstring ignored;
            (void)account->bridge.Call(Command::StopPath, 0, 0, 0, response, ignored, 700);
            (void)account->bridge.Call(Command::SetAutoFight, 0, 0, 0, response, ignored, 1200);
        }
        if (account) {
            account->runtime.running = false;
            ResetRuntime(account->runtime);
            account->runtime.running = false;
            account->runtime.status = failure ? L"PHÓ BẢN LỖI" : L"Đã dừng";
            LogAccount(*account, (failure ? L"AUTO PHÓ BẢN FAIL-CLOSED: " : L"AUTO PHÓ BẢN DỪNG: ") + reason);
        }
        dungeonRuntime_.running = false;
        dungeonRuntime_.phase = failure ? DungeonPhase::Error : DungeonPhase::Idle;
        dungeonRuntime_.status = (failure ? L"LỖI • " : L"DỪNG • ") + reason;
        if (dungeonStatus_) SetText(dungeonStatus_, L"AUTO PHÓ BẢN: " + dungeonRuntime_.status);
        RefreshDungeonStageList();
    }

    void FailDungeon(const std::wstring& reason) { StopDungeonInternal(reason, true); }

    void StopDungeonUser() {
        if (!dungeonRuntime_.running) {
            SetDungeonRuntimeStatus(L"Đã dừng"); return;
        }
        StopDungeonInternal(L"người dùng bấm DỪNG", false);
    }

    void StartDungeon() {
        SaveDungeonProfileHeader();
        DungeonProfile* profile = CurrentDungeonProfile();
        Account* account = SelectedDungeonAccount();
        if (!profile || !account) {
            MessageBoxW(hwnd_, L"Hãy chọn acc và hồ sơ phó bản.", kTitle, MB_OK | MB_ICONWARNING);
            return;
        }
        std::wstring error;
        if (!ValidateDungeonProfile(*profile, error)) {
            MessageBoxW(hwnd_, (L"Chưa thể chạy:\n" + error).c_str(), kTitle, MB_OK | MB_ICONWARNING);
            return;
        }
        if (profile->entryClicks.empty() &&
            MessageBoxW(hwnd_, L"Hồ sơ chưa có CHUỖI VÀO. ClickNPC chỉ mở NPC; nếu dialog không tự vào map thì workflow sẽ timeout. Vẫn chạy?",
                        kTitle, MB_YESNO | MB_ICONWARNING) != IDYES) return;

        if (dungeonRuntime_.running) StopDungeonInternal(L"khởi động lại bằng hồ sơ mới", false);
        const bool autoTrainRunning = std::any_of(accounts_.begin(), accounts_.end(), [](const std::unique_ptr<Account>& item) {
            return item->runtime.running;
        });
        if (autoTrainRunning) {
            if (MessageBoxW(hwnd_, L"AUTO TRAIN đang chạy. Dừng toàn bộ AUTO TRAIN để chuyển sang AUTO PHÓ BẢN?\n\nChỉ một trong hai tab được phép chạy.",
                            kTitle, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
            if (tradeTxn_.phase != TradePhase::Idle) AbortTrade(L"chuyển sang AUTO PHÓ BẢN", GetTickCount());
            ReleaseTradeHolds();
            for (auto& item : accounts_) if (item->runtime.running) StopAccount(*item);
        }
        if (!EnsureAttach(*account, error)) {
            MessageBoxW(hwnd_, (L"Không attach được acc: " + error).c_str(), kTitle, MB_OK | MB_ICONERROR);
            return;
        }

        account->deathSessionLatched = false;
        account->tradeHeld = false;
        account->runtime.running = true;
        ResetRuntime(account->runtime);
        account->runtime.running = true;
        dungeonRuntime_ = DungeonRuntime{};
        dungeonRuntime_.running = true;
        dungeonRuntime_.pid = account->game.pid;
        dungeonRuntime_.profileIndex = dungeonProfileIndex_;
        dungeonActiveProfile_ = *profile; // Freeze the exact configuration for this run/cycle.
        dungeonRuntime_.cycle = 1;
        dungeonRuntime_.stageIndex = 0;
        dungeonRuntime_.stageKillCounts.assign(profile->stages.size(), 0);
        dungeonRuntime_.deathTracker.Reset(profile->gatherMapID, -1);
        BeginDungeonPhase(DungeonPhase::TravelNpc,
                          L"Vòng 1 • đi NPC ResID " + std::to_wstring(profile->npcResID), GetTickCount());
        LogAccount(*account, L"BẮT ĐẦU AUTO PHÓ BẢN • " + profile->name + L" • NPC ResID " +
                           std::to_wstring(profile->npcResID) + L" • map phó bản " +
                           std::to_wstring(profile->dungeonMapID) + L" • " +
                           std::to_wstring(profile->stages.size()) + L" bước.");
    }

    static bool DungeonAtTarget(const Snapshot& snapshot, int mapID, int x, int y, int tolerance) {
        if ((snapshot.validMask & (ValidMap | ValidPosition)) != (ValidMap | ValidPosition) ||
            !snapshot.mapReady || snapshot.waitingChangeMap || snapshot.mapID != mapID) return false;
        const std::int64_t dx = static_cast<std::int64_t>(snapshot.x) - x;
        const std::int64_t dy = static_cast<std::int64_t>(snapshot.y) - y;
        return dx * dx + dy * dy <= static_cast<std::int64_t>(tolerance) * tolerance;
    }

    bool DungeonSetFight(Account& account, bool desired, DWORD now, const std::wstring& context) {
        if ((account.snapshot.validMask & ValidAutoFight) == 0) {
            SetDungeonRuntimeStatus(context + L" • chờ getter AutoFight authoritative");
            return false;
        }
        if ((account.snapshot.autoFight != 0) == desired) {
            return true;
        }
        if (dungeonRuntime_.lastCommandTick != 0 && now - dungeonRuntime_.lastCommandTick < 1400) return false;
        Response response{}; std::wstring error;
        const bool ok = account.bridge.Call(Command::SetAutoFight, desired ? 1 : 0, 0, 0,
                                            response, error, 1700);
        dungeonRuntime_.lastCommandTick = now;
        ++dungeonRuntime_.commandAttempts;
        if (!ok) LogAccount(account, L"PHÓ BẢN SetAutoFight fail-closed: " + error);
        SetDungeonRuntimeStatus(context + (desired ? L" • đã gửi Train=1, chờ verify ON" : L" • đã gửi None=0, chờ verify OFF"));
        if (dungeonRuntime_.commandAttempts >= 5) {
            FailDungeon(L"SetAutoFight không đạt state " + std::wstring(desired ? L"ON" : L"OFF") +
                        L" sau 5 lần • " + error);
        }
        return false;
    }

    bool DungeonTravel(Account& account, int mapID, int x, int y, int tolerance,
                       const std::wstring& label, DWORD now) {
        if (DungeonAtTarget(account.snapshot, mapID, x, y, tolerance)) {
            if (account.snapshot.autoPathing &&
                (dungeonRuntime_.lastCommandTick == 0 || now - dungeonRuntime_.lastCommandTick >= 1000)) {
                Response response{}; std::wstring ignored;
                (void)account.bridge.Call(Command::StopPath, 0, 0, 0, response, ignored, 900);
                dungeonRuntime_.lastCommandTick = now;
                SetDungeonRuntimeStatus(L"Đã tới " + label + L" • chờ StopPath authoritative");
                return false;
            }
            return !account.snapshot.autoPathing;
        }
        if (!DungeonSetFight(account, false, now, L"TRAVEL GUARD " + label)) return false;
        if (dungeonRuntime_.lastCommandTick != 0 && now - dungeonRuntime_.lastCommandTick < 3000) return false;
        Response response{}; std::wstring error;
        const bool ok = account.bridge.Call(Command::StartPath, mapID, x, y, response, error, 1500);
        dungeonRuntime_.lastCommandTick = now;
        if (!ok) {
            ++dungeonRuntime_.commandAttempts;
            LogAccount(account, L"PHÓ BẢN StartPath " + label + L" FAIL: " + error);
            if (dungeonRuntime_.commandAttempts >= 5) FailDungeon(L"Không StartPath được tới " + label + L" • " + error);
        } else {
            dungeonRuntime_.commandAttempts = 0;
            if (mapID != account.snapshot.mapID) {
                if (!account.runtime.crossMapRouteArmed) account.runtime.crossMapRouteMoved = false;
                account.runtime.crossMapRouteArmed = true;
            }
            SetDungeonRuntimeStatus(L"Đang AutoPath tới " + label + L" • M" + std::to_wstring(mapID) +
                                    L" • " + std::to_wstring(x) + L"," + std::to_wstring(y));
        }
        return false;
    }

    bool RunDungeonClickSequence(Account& account, const std::vector<DungeonClickStep>& sequence,
                                 bool entering, DWORD now, const std::wstring& label) {
        DungeonProfile* profile = dungeonRuntime_.running ? &dungeonActiveProfile_ : nullptr;
        if (profile && ((entering && account.snapshot.mapID == profile->dungeonMapID) ||
                        (!entering && account.snapshot.mapID != profile->dungeonMapID))) {
            ReleaseDungeonClickLease(label + L" • map đã đổi"); return true;
        }
        if (dungeonRuntime_.clickIndex >= sequence.size()) {
            ReleaseDungeonClickLease(label + L" • hoàn tất"); return true;
        }
        if (!dungeonRuntime_.clickLeaseHeld) {
            if (!AcquireCoordinatorSequenceLease(account, L"AUTO PHÓ BẢN • " + label)) {
                SetDungeonRuntimeStatus(label + L" • chờ click lease"); return false;
            }
            dungeonRuntime_.clickLeaseHeld = true;
        }
        const DungeonClickStep& step = sequence[dungeonRuntime_.clickIndex];
        if (!step.point.valid) {
            FailDungeon(label + L" có dòng chưa lấy tọa #" + std::to_wstring(dungeonRuntime_.clickIndex + 1));
            return false;
        }
        if (dungeonRuntime_.lastCommandTick != 0 &&
            now - dungeonRuntime_.lastCommandTick < static_cast<DWORD>(step.delayMs)) return false;
        POINT point{}; std::wstring error;
        if (!ScaleClickPoint(account.game, step.point, point, error) ||
            !CoordinatorClick(account, point, L"AUTO PHÓ BẢN " + label + L" #" +
                              std::to_wstring(dungeonRuntime_.clickIndex + 1), error)) {
            if (error.rfind(L"CHUỘT TAY", 0) == 0) {
                SetDungeonRuntimeStatus(label + L" • chuột tay, giữ nguyên dòng và chờ 5s");
                return false;
            }
            FailDungeon(label + L" click FAIL: " + error); return false;
        }
        dungeonRuntime_.lastCommandTick = now;
        ++dungeonRuntime_.clickRepeatDone;
        if (dungeonRuntime_.clickRepeatDone >= step.repeat) {
            dungeonRuntime_.clickRepeatDone = 0;
            ++dungeonRuntime_.clickIndex;
        }
        SetDungeonRuntimeStatus(label + L" • dòng " + std::to_wstring(dungeonRuntime_.clickIndex + 1) +
                                L" / " + std::to_wstring(sequence.size()));
        return false;
    }

    void UpdateDungeonCounterFromScan(Account& account, const DungeonProfile& profile,
                                      const DungeonStage& stage, const Response& response) {
        std::vector<MonsterObservation> observations;
        observations.reserve(response.monsterCount);
        for (std::uint32_t i = 0; i < response.monsterCount; ++i) {
            const MonsterRecord& record = response.monsters[i];
            MonsterObservation observation{};
            observation.roleID = record.roleID;
            observation.resID = record.resID;
            observation.hp = record.hp;
            observation.maxHP = record.maxHP;
            observation.x = record.x;
            observation.y = record.y;
            observation.dead = record.dead != 0;
            observation.positionValid = (record.validMask & MonsterValidPosition) != 0;
            observation.name = record.name;
            observations.push_back(std::move(observation));
        }
        const std::vector<KillEvent> events = dungeonRuntime_.deathTracker.Observe(
            observations, profile.rules, stage.filterGroup, stage.x, stage.y, stage.countRadius);
        if (!events.empty()) {
            dungeonRuntime_.stageKills += static_cast<int>(events.size());
            dungeonRuntime_.totalKills += static_cast<int>(events.size());
            if (dungeonRuntime_.stageIndex >= 0 &&
                dungeonRuntime_.stageIndex < static_cast<int>(dungeonRuntime_.stageKillCounts.size())) {
                dungeonRuntime_.stageKillCounts[static_cast<std::size_t>(dungeonRuntime_.stageIndex)] =
                    dungeonRuntime_.stageKills;
            }
            for (const KillEvent& event : events) {
                LogAccount(account, L"KILL +1 • " + (event.name.empty() ? L"?" : event.name) +
                                   L" • RoleID " + std::to_wstring(event.roleID) +
                                   L" • ResID " + std::to_wstring(event.resID) +
                                   L" • bước " + stage.name + L" = " +
                                   std::to_wstring(dungeonRuntime_.stageKills) + L"/" +
                                   std::to_wstring(stage.requiredKills));
            }
            RefreshDungeonStageList();
        }
    }

    void TickDungeonAccount(Account& account, DWORD now) {
        if (!dungeonRuntime_.running || dungeonRuntime_.pid != account.game.pid) return;
        DungeonProfile& profile = dungeonActiveProfile_;
        const Snapshot& snapshot = account.snapshot;
        if ((snapshot.validMask & ValidLifeState) && snapshot.dead) {
            FailDungeon(L"Nhân vật chết trong workflow; dừng để không suy diễn bước/map sau hồi sinh"); return;
        }
        if (!snapshot.mapReady || snapshot.waitingChangeMap) {
            SetDungeonRuntimeStatus(L"Đang chuyển map • FREEZE action, chờ state ổn định"); return;
        }

        switch (dungeonRuntime_.phase) {
            case DungeonPhase::TravelNpc:
                if (now - dungeonRuntime_.phaseTick > 300000) { FailDungeon(L"Timeout 300s đi tới NPC"); return; }
                if (DungeonTravel(account, profile.gatherMapID, profile.npcX, profile.npcY, 140, L"NPC phó bản", now))
                    BeginDungeonPhase(DungeonPhase::ClickNpc, L"Đã tới NPC • chuẩn bị ClickNPC ResID", now);
                return;

            case DungeonPhase::ClickNpc: {
                if (now - dungeonRuntime_.phaseTick < 500) return;
                Response response{}; std::wstring error;
                if (!account.bridge.Call(Command::ClickNpc, profile.npcResID, 0, 0, response, error, 1300)) {
                    ++dungeonRuntime_.commandAttempts;
                    if (dungeonRuntime_.commandAttempts >= 5) FailDungeon(L"ClickNPC ResID fail: " + error);
                    return;
                }
                BeginDungeonPhase(DungeonPhase::EntryClicks, L"NPC đã mở • chạy CHUỖI VÀO", now);
                return;
            }

            case DungeonPhase::EntryClicks:
                if (now - dungeonRuntime_.phaseTick < 650) return;
                if (RunDungeonClickSequence(account, profile.entryClicks, true, now, L"CHUỖI VÀO"))
                    BeginDungeonPhase(DungeonPhase::WaitDungeonMap, L"Chờ MapID phó bản " +
                                      std::to_wstring(profile.dungeonMapID), now);
                return;

            case DungeonPhase::WaitDungeonMap:
                if (snapshot.mapID == profile.dungeonMapID) {
                    dungeonRuntime_.stageIndex = 0;
                    dungeonRuntime_.stageKills = 0;
                    dungeonRuntime_.stageKillCounts.assign(profile.stages.size(), 0);
                    dungeonRuntime_.deathTracker.Reset(snapshot.mapID, 0);
                    BeginDungeonPhase(DungeonPhase::TravelStage, L"Đã vào map • đi bước 1", now);
                } else if (now - dungeonRuntime_.phaseTick > static_cast<DWORD>(profile.entryTimeoutSec) * 1000u) {
                    FailDungeon(L"Timeout vào map; hãy sửa CHUỖI VÀO hoặc Dungeon MapID");
                }
                return;

            case DungeonPhase::TravelStage: {
                if (dungeonRuntime_.stageIndex < 0 || dungeonRuntime_.stageIndex >= static_cast<int>(profile.stages.size())) {
                    FailDungeon(L"Stage index vượt cấu hình"); return;
                }
                DungeonStage& stage = profile.stages[static_cast<std::size_t>(dungeonRuntime_.stageIndex)];
                if (snapshot.mapID != stage.mapID) {
                    FailDungeon(L"Sai map ở bước " + stage.name + L": đang M" + std::to_wstring(snapshot.mapID) +
                                L", cần M" + std::to_wstring(stage.mapID)); return;
                }
                if (now - dungeonRuntime_.phaseTick > 300000) { FailDungeon(L"Timeout 300s đi tới " + stage.name); return; }
                if (DungeonTravel(account, stage.mapID, stage.x, stage.y, stage.tolerance, stage.name, now))
                    BeginDungeonPhase(DungeonPhase::StartFight, L"Đã tới " + stage.name + L" • bật AutoFight", now);
                return;
            }

            case DungeonPhase::StartFight: {
                DungeonStage& stage = profile.stages[static_cast<std::size_t>(dungeonRuntime_.stageIndex)];
                if (DungeonSetFight(account, true, now, L"BẬT AUTO tại " + stage.name)) {
                    dungeonRuntime_.stageKills = 0;
                    dungeonRuntime_.deathTracker.Reset(snapshot.mapID, dungeonRuntime_.stageIndex);
                    BeginDungeonPhase(DungeonPhase::Fighting,
                                      stage.name + L" • 0/" + std::to_wstring(stage.requiredKills), now);
                }
                return;
            }

            case DungeonPhase::Fighting: {
                DungeonStage& stage = profile.stages[static_cast<std::size_t>(dungeonRuntime_.stageIndex)];
                if (snapshot.mapID != stage.mapID) { FailDungeon(L"Đổi map ngoài dự kiến khi đang đếm " + stage.name); return; }
                if (now - dungeonRuntime_.phaseTick > static_cast<DWORD>(stage.timeoutSec) * 1000u) {
                    FailDungeon(L"Timeout bước " + stage.name + L" tại " +
                                std::to_wstring(dungeonRuntime_.stageKills) + L"/" +
                                std::to_wstring(stage.requiredKills)); return;
                }
                if ((snapshot.validMask & ValidAutoFight) && !snapshot.autoFight) {
                    (void)DungeonSetFight(account, true, now, L"AutoFight rơi OFF tại " + stage.name);
                    return;
                }
                if (dungeonRuntime_.lastScanTick == 0 || now - dungeonRuntime_.lastScanTick >= 500) {
                    Response response{}; std::wstring error;
                    dungeonRuntime_.lastScanTick = now;
                    if (!ScanDungeonMonsters(account, response, error)) {
                        ++dungeonRuntime_.commandAttempts;
                        SetDungeonRuntimeStatus(stage.name + L" • scanner lỗi " + std::to_wstring(dungeonRuntime_.commandAttempts) + L"/5 • " + error);
                        if (dungeonRuntime_.commandAttempts >= 5) FailDungeon(L"Scanner quái/HP lỗi 5 lần: " + error);
                        return;
                    }
                    dungeonRuntime_.commandAttempts = 0;
                    dungeonRuntime_.lastScan.assign(response.monsters, response.monsters + response.monsterCount);
                    if (mainTabIndex_ == 1) RefreshDungeonScanList(response);
                    UpdateDungeonCounterFromScan(account, profile, stage, response);
                }
                if (StageComplete(dungeonRuntime_.stageKills, stage.requiredKills)) {
                    BeginDungeonPhase(DungeonPhase::StopFight,
                                      stage.name + L" đủ " + std::to_wstring(dungeonRuntime_.stageKills) +
                                      L" • tắt AutoFight", now);
                } else {
                    SetDungeonRuntimeStatus(L"Vòng " + std::to_wstring(dungeonRuntime_.cycle) + L" • " + stage.name +
                                            L" • " + std::to_wstring(dungeonRuntime_.stageKills) + L"/" +
                                            std::to_wstring(stage.requiredKills) + L" • quét mỗi 500ms");
                }
                return;
            }

            case DungeonPhase::StopFight:
                if (DungeonSetFight(account, false, now, L"TẮT AUTO sau bước")) {
                    if (dungeonRuntime_.stageIndex + 1 < static_cast<int>(profile.stages.size())) {
                        ++dungeonRuntime_.stageIndex;
                        dungeonRuntime_.stageKills = 0;
                        dungeonRuntime_.deathTracker.Reset(snapshot.mapID, dungeonRuntime_.stageIndex);
                        BeginDungeonPhase(DungeonPhase::TravelStage,
                                          L"Đi bước " + std::to_wstring(dungeonRuntime_.stageIndex + 1), now);
                    } else {
                        BeginDungeonPhase(DungeonPhase::ExitClicks, L"Đã xong boss/bước cuối • CHUỖI RA", now);
                    }
                }
                return;

            case DungeonPhase::ExitClicks:
                if (RunDungeonClickSequence(account, profile.exitClicks, false, now, L"CHUỖI RA"))
                    BeginDungeonPhase(DungeonPhase::WaitExitMap, L"Chờ rời MapID " +
                                      std::to_wstring(profile.dungeonMapID), now);
                return;

            case DungeonPhase::WaitExitMap:
                if (snapshot.mapID != profile.dungeonMapID) {
                    if (profile.loop) {
                        BeginDungeonPhase(DungeonPhase::LoopDelay,
                                          L"Vòng " + std::to_wstring(dungeonRuntime_.cycle) +
                                          L" hoàn tất • chờ vòng mới", now);
                    } else {
                        dungeonRuntime_.phase = DungeonPhase::Complete;
                        StopDungeonInternal(L"Hoàn tất 1 vòng (không lặp)", false);
                    }
                } else if (now - dungeonRuntime_.phaseTick > static_cast<DWORD>(profile.exitTimeoutSec) * 1000u) {
                    FailDungeon(L"Timeout rời map; hãy cấu hình CHUỖI RA hoặc cơ chế auto eject");
                }
                return;

            case DungeonPhase::LoopDelay:
                if (now - dungeonRuntime_.phaseTick >= 3000) {
                    ++dungeonRuntime_.cycle;
                    dungeonRuntime_.stageIndex = 0;
                    dungeonRuntime_.stageKills = 0;
                    dungeonRuntime_.stageKillCounts.assign(profile.stages.size(), 0);
                    dungeonRuntime_.deathTracker.Reset(snapshot.mapID, -1);
                    BeginDungeonPhase(DungeonPhase::TravelNpc,
                                      L"Vòng " + std::to_wstring(dungeonRuntime_.cycle) + L" • quay lại NPC", now);
                }
                return;

            case DungeonPhase::Complete:
            case DungeonPhase::Error:
            case DungeonPhase::Idle:
                return;
        }
    }

    std::vector<DungeonClickStep>* DungeonEditorSequence() {
        DungeonProfile* profile = CurrentDungeonProfile();
        if (!profile) return nullptr;
        return dungeonSequenceExit_ ? &profile->exitClicks : &profile->entryClicks;
    }

    void RefreshDungeonSequenceList() {
        if (!dungeonSequenceList_) return;
        const int keep = ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED);
        ListView_DeleteAllItems(dungeonSequenceList_);
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence();
        if (!sequence) return;
        for (std::size_t i = 0; i < sequence->size(); ++i) {
            const DungeonClickStep& step = (*sequence)[i];
            const std::wstring index = std::to_wstring(i + 1);
            LVITEMW item{}; item.mask = LVIF_TEXT; item.iItem = static_cast<int>(i);
            item.pszText = const_cast<wchar_t*>(index.c_str()); ListView_InsertItem(dungeonSequenceList_, &item);
            ListView_SetItemText(dungeonSequenceList_, static_cast<int>(i), 1, const_cast<wchar_t*>(step.description.c_str()));
            const std::wstring point = PointDescription(step.point);
            const std::wstring delay = std::to_wstring(step.delayMs);
            const std::wstring repeat = std::to_wstring(step.repeat);
            ListView_SetItemText(dungeonSequenceList_, static_cast<int>(i), 2, const_cast<wchar_t*>(point.c_str()));
            ListView_SetItemText(dungeonSequenceList_, static_cast<int>(i), 3, const_cast<wchar_t*>(delay.c_str()));
            ListView_SetItemText(dungeonSequenceList_, static_cast<int>(i), 4, const_cast<wchar_t*>(repeat.c_str()));
        }
        if (!sequence->empty()) {
            const int row = std::clamp(keep < 0 ? 0 : keep, 0, static_cast<int>(sequence->size()) - 1);
            ListView_SetItemState(dungeonSequenceList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }

    void LoadDungeonSequenceEditorRow(int row = -1) {
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence();
        if (!sequence || sequence->empty()) return;
        if (row < 0) row = ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(sequence->size())) return;
        const DungeonClickStep& step = (*sequence)[static_cast<std::size_t>(row)];
        SetText(dungeonSequenceDesc_, step.description);
        SetText(dungeonSequenceDelay_, std::to_wstring(step.delayMs));
        SetText(dungeonSequenceRepeat_, std::to_wstring(step.repeat));
    }

    void SaveDungeonSequenceRow() {
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence(); if (!sequence) return;
        int row = ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(sequence->size())) return;
        DungeonClickStep& step = (*sequence)[static_cast<std::size_t>(row)];
        step.description = SanitizeSpotName(GetText(dungeonSequenceDesc_));
        if (step.description.empty()) step.description = L"Click " + std::to_wstring(row + 1);
        step.delayMs = std::clamp(_wtoi(GetText(dungeonSequenceDelay_).c_str()), 50, 60000);
        step.repeat = std::clamp(_wtoi(GetText(dungeonSequenceRepeat_).c_str()), 1, 999);
        SaveDungeonProfiles(); RefreshDungeonSequenceList();
        ListView_SetItemState(dungeonSequenceList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonSequenceEditorRow(row); LoadDungeonProfileToUi();
    }

    void AddDungeonSequenceRow() {
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence(); if (!sequence) return;
        DungeonClickStep step{}; step.description = L"Click " + std::to_wstring(sequence->size() + 1);
        sequence->push_back(std::move(step)); SaveDungeonProfiles(); RefreshDungeonSequenceList();
        const int row = static_cast<int>(sequence->size()) - 1;
        ListView_SetItemState(dungeonSequenceList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonSequenceEditorRow(row); LoadDungeonProfileToUi();
    }

    void DeleteDungeonSequenceRow() {
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence(); if (!sequence) return;
        const int row = ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED);
        if (row < 0 || row >= static_cast<int>(sequence->size())) return;
        sequence->erase(sequence->begin() + row); SaveDungeonProfiles(); RefreshDungeonSequenceList();
        LoadDungeonSequenceEditorRow(); LoadDungeonProfileToUi();
    }

    void MoveDungeonSequenceRow(int direction) {
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence(); if (!sequence) return;
        const int row = ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED);
        const int target = row + direction;
        if (row < 0 || target < 0 || row >= static_cast<int>(sequence->size()) || target >= static_cast<int>(sequence->size())) return;
        std::swap((*sequence)[static_cast<std::size_t>(row)], (*sequence)[static_cast<std::size_t>(target)]);
        SaveDungeonProfiles(); RefreshDungeonSequenceList();
        ListView_SetItemState(dungeonSequenceList_, target, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadDungeonSequenceEditorRow(target);
    }

    void BeginDungeonSequenceCapture() {
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence();
        Account* account = SelectedDungeonAccount();
        const int row = dungeonSequenceList_ ? ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED) : -1;
        if (!sequence || !account || row < 0 || row >= static_cast<int>(sequence->size())) {
            Log(L"[AUTO PHÓ BẢN] Chọn acc và một dòng chuỗi để LẤY TỌA."); return;
        }
        SaveDungeonSequenceRow();
        captureSlot_ = ClickSlot::None;
        captureMacroIndex_ = -1;
        captureTradeSequenceIndex_ = -1;
        captureDungeonSequenceIndex_ = row;
        captureDungeonSequenceExit_ = dungeonSequenceExit_;
        captureDungeonProfileIndex_ = dungeonProfileIndex_;
        capturePid_ = account->game.pid;
        LogAccount(*account, L"AUTO PHÓ BẢN chờ F8 cho " +
                            std::wstring(dungeonSequenceExit_ ? L"CHUỖI RA" : L"CHUỖI VÀO") +
                            L" dòng " + std::to_wstring(row + 1));
    }

    void TestDungeonSequenceRow() {
        SaveDungeonSequenceRow();
        std::vector<DungeonClickStep>* sequence = DungeonEditorSequence();
        Account* account = SelectedDungeonAccount();
        const int row = dungeonSequenceList_ ? ListView_GetNextItem(dungeonSequenceList_, -1, LVNI_SELECTED) : -1;
        if (!sequence || !account || row < 0 || row >= static_cast<int>(sequence->size())) return;
        const DungeonClickStep& step = (*sequence)[static_cast<std::size_t>(row)];
        POINT point{}; std::wstring error;
        if (!ScaleClickPoint(account->game, step.point, point, error) ||
            !CoordinatorClick(*account, point, L"TEST CHUỖI PHÓ BẢN", error, false)) {
            LogAccount(*account, L"TEST CHUỖI PHÓ BẢN FAIL: " + error); return;
        }
        LogAccount(*account, L"TEST CHUỖI PHÓ BẢN PASS dòng " + std::to_wstring(row + 1));
    }

    void OpenDungeonSequenceEditor(bool exitSequence) {
        if (!CurrentDungeonProfile()) return;
        if (dungeonSequenceEditor_ && IsWindow(dungeonSequenceEditor_)) DestroyWindow(dungeonSequenceEditor_);
        dungeonSequenceExit_ = exitSequence;
        WNDCLASSEXW wc{}; wc.cbSize = sizeof(wc); wc.lpfnWndProc = DungeonSequenceWndProc;
        wc.hInstance = instance_; wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"ThanLongDungeonSequenceEditorV06";
        if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            Log(L"Không đăng ký được editor chuỗi phó bản."); return;
        }
        const wchar_t* title = exitSequence ? L"AUTO PHÓ BẢN • CHUỖI RA" : L"AUTO PHÓ BẢN • CHUỖI VÀO";
        dungeonSequenceEditor_ = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, title,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, 780, 430, hwnd_, nullptr, instance_, this);
        if (!dungeonSequenceEditor_) return;
        auto column = [&](int index, int width, const wchar_t* text) {
            LVCOLUMNW c{}; c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
            c.pszText = const_cast<wchar_t*>(text); c.cx = width; c.iSubItem = index;
            ListView_InsertColumn(dungeonSequenceList_, index, &c);
        };
        MakeIn(dungeonSequenceEditor_, L"STATIC",
               exitSequence ? L"CHUỖI RA — chạy sau khi boss/bước cuối đủ kill" : L"CHUỖI VÀO — chạy sau ClickNPC(ResID)",
               0, 15, 10, 730, 24, 0);
        dungeonSequenceList_ = MakeIn(dungeonSequenceEditor_, WC_LISTVIEWW, L"",
            LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER, 15, 40, 730, 210, IDC_DGSEQ_LIST);
        ListView_SetExtendedListViewStyle(dungeonSequenceList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        column(0, 35, L"#"); column(1, 255, L"Mô tả"); column(2, 230, L"Tọa");
        column(3, 90, L"Delay"); column(4, 70, L"Lặp");
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"+ THÊM", BS_PUSHBUTTON, 15, 260, 80, 28, IDC_DGSEQ_ADD);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"- XÓA", BS_PUSHBUTTON, 101, 260, 70, 28, IDC_DGSEQ_DELETE);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"LÊN", BS_PUSHBUTTON, 177, 260, 55, 28, IDC_DGSEQ_UP);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"XUỐNG", BS_PUSHBUTTON, 238, 260, 60, 28, IDC_DGSEQ_DOWN);
        dungeonSequenceDesc_ = MakeIn(dungeonSequenceEditor_, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL,
                                      306, 260, 215, 28, IDC_DGSEQ_DESC);
        dungeonSequenceDelay_ = MakeIn(dungeonSequenceEditor_, L"EDIT", L"600", WS_BORDER | ES_NUMBER | ES_CENTER,
                                       527, 260, 70, 28, IDC_DGSEQ_DELAY);
        dungeonSequenceRepeat_ = MakeIn(dungeonSequenceEditor_, L"EDIT", L"1", WS_BORDER | ES_NUMBER | ES_CENTER,
                                        603, 260, 45, 28, IDC_DGSEQ_REPEAT);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"LƯU", BS_PUSHBUTTON, 654, 260, 91, 28, IDC_DGSEQ_SAVE);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"LẤY TỌA (F8)", BS_PUSHBUTTON, 15, 298, 135, 30, IDC_DGSEQ_CAPTURE);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"TEST DÒNG", BS_PUSHBUTTON, 158, 298, 110, 30, IDC_DGSEQ_TEST);
        MakeIn(dungeonSequenceEditor_, L"BUTTON", L"ĐÓNG", BS_PUSHBUTTON, 655, 298, 90, 30, IDC_DGSEQ_CLOSE);
        MakeIn(dungeonSequenceEditor_, L"STATIC", L"REAL CLICK được scale theo kích thước client. User Mouse Guard vẫn pause 5 giây.",
               0, 15, 338, 730, 24, 0);
        RefreshDungeonSequenceList(); LoadDungeonSequenceEditorRow();
        ShowWindow(dungeonSequenceEditor_, SW_SHOW); UpdateWindow(dungeonSequenceEditor_);
    }

    LRESULT HandleDungeonSequenceEditor(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_NOTIFY: {
                const auto* hdr = reinterpret_cast<const NMHDR*>(lp);
                if (hdr && hdr->idFrom == IDC_DGSEQ_LIST && hdr->code == LVN_ITEMCHANGED) {
                    const auto* change = reinterpret_cast<const NMLISTVIEW*>(hdr);
                    if ((change->uChanged & LVIF_STATE) && (change->uNewState & LVIS_SELECTED))
                        LoadDungeonSequenceEditorRow(change->iItem);
                }
                return 0;
            }
            case WM_COMMAND:
                switch (LOWORD(wp)) {
                    case IDC_DGSEQ_ADD: AddDungeonSequenceRow(); return 0;
                    case IDC_DGSEQ_DELETE: DeleteDungeonSequenceRow(); return 0;
                    case IDC_DGSEQ_UP: MoveDungeonSequenceRow(-1); return 0;
                    case IDC_DGSEQ_DOWN: MoveDungeonSequenceRow(1); return 0;
                    case IDC_DGSEQ_SAVE: SaveDungeonSequenceRow(); return 0;
                    case IDC_DGSEQ_CAPTURE: BeginDungeonSequenceCapture(); return 0;
                    case IDC_DGSEQ_TEST: TestDungeonSequenceRow(); return 0;
                    case IDC_DGSEQ_CLOSE: DestroyWindow(hwnd); return 0;
                }
                break;
            case WM_CLOSE: DestroyWindow(hwnd); return 0;
            case WM_NCDESTROY:
                dungeonSequenceEditor_ = nullptr; dungeonSequenceList_ = nullptr;
                dungeonSequenceDesc_ = nullptr; dungeonSequenceDelay_ = nullptr; dungeonSequenceRepeat_ = nullptr;
                captureDungeonSequenceIndex_ = -1;
                captureDungeonProfileIndex_ = -1;
                return DefWindowProcW(hwnd, msg, wp, lp);
        }
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
