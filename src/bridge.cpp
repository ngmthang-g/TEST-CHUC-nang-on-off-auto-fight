#include "bridge_runtime.inl"
#include "bridge_lua.inl"

bool SetAutoFight(bool enabled, wchar_t* detail, std::size_t cap) {
    Il2CppClass* gameApi = nullptr;
    Il2CppClass* session = nullptr;
    Il2CppClass* luaManager = nullptr;
    if (!ResolveCore(gameApi, session, luaManager, detail, cap)) return false;
    if (!SafeForAction(gameApi, session, detail, cap)) return false;

    // VERIFIED shipped semantic wrappers from the DATA knowledge base:
    // TopIcon:AutoTrainClick() -> AutoFight_Main:StartAutoFight(C_AutoModel.Train)
    // TopIcon:AutoStopClick()  -> AutoFight_Main:StartAutoFight(C_AutoModel.None)
    return ExecuteLuaNoArg(luaManager, "TopIcon", enabled ? "AutoTrainClick" : "AutoStopClick", detail, cap);
}

bool EnsureShared() {
    if (g_shared) return true;
    wchar_t name[96]{};
    MappingName(GetCurrentProcessId(), name, _countof(name));
    g_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!g_mapping) return false;
    g_shared = reinterpret_cast<SharedBlock*>(MapViewOfFile(
        g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    if (!g_shared || g_shared->magic != kMagic ||
        g_shared->protocolVersion != kProtocolVersion ||
        g_shared->targetPid != GetCurrentProcessId()) {
        if (g_shared) UnmapViewOfFile(g_shared);
        if (g_mapping) CloseHandle(g_mapping);
        g_shared = nullptr;
        g_mapping = nullptr;
        return false;
    }
    InterlockedExchange(&g_shared->bridgeLoaded, 1);
    return true;
}

void ProcessRequest() {
    if (!EnsureShared()) return;
    const LONG seq = g_shared->requestSeq;
    if (seq <= 0 || seq == g_shared->completedSeq) return;
    if (InterlockedCompareExchange(&g_shared->bridgeBusy, 1, 0) != 0) return;

    Response response{};
    response.callbackThreadId = GetCurrentThreadId();
    wchar_t detail[512]{};
    bool ok = false;

    if (response.callbackThreadId != g_shared->targetWindowThreadId) {
        SetText(detail, _countof(detail), L"Sai callback thread; action bị chặn");
        response.errorCode = 1001;
    } else {
        switch (static_cast<Command>(g_shared->request.command)) {
            case Command::Probe:
                ok = true;
                SetText(detail, _countof(detail), L"Bridge probe PASS; chưa gửi AutoFight action");
                break;
            case Command::StartAutoFight:
                ok = SetAutoFight(true, detail, _countof(detail));
                response.errorCode = ok ? 0 : 1501;
                break;
            case Command::StopAutoFight:
                ok = SetAutoFight(false, detail, _countof(detail));
                response.errorCode = ok ? 0 : 1601;
                break;
            default:
                SetText(detail, _countof(detail), L"Command không hợp lệ");
                response.errorCode = 1002;
                break;
        }
    }

    response.ok = ok ? 1 : 0;
    SetText(response.detail, _countof(response.detail), detail);
    g_shared->response = response;
    MemoryBarrier();
    InterlockedExchange(&g_shared->completedSeq, seq);
    InterlockedExchange(&g_shared->bridgeBusy, 0);
}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK TlcGetMessageHook(int code, WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    if (code >= 0 && lParam) {
        const MSG* msg = reinterpret_cast<const MSG*>(lParam);
        if (msg->message == kWakeMessage) ProcessRequest();
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_shared) UnmapViewOfFile(g_shared);
        if (g_mapping) CloseHandle(g_mapping);
        g_shared = nullptr;
        g_mapping = nullptr;
    }
    return TRUE;
}
