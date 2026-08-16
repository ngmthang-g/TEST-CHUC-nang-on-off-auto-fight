struct ActionApi {
    const Il2CppImage* (__cdecl* get_corlib)() = nullptr;
    const char* (__cdecl* class_get_name)(Il2CppClass*) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    Il2CppObject* (__cdecl* value_box)(Il2CppClass*, void*) = nullptr;
    std::uint32_t (__cdecl* gchandle_new)(Il2CppObject*, bool) = nullptr;
    Il2CppObject* (__cdecl* gchandle_get_target)(std::uint32_t) = nullptr;
    void (__cdecl* gchandle_free)(std::uint32_t) = nullptr;

    bool Load(wchar_t* detail, std::size_t cap) {
        if (get_corlib) return true;
#define ACTION_NEED(symbol) do { \
    if (!Resolve(g_api.module, "il2cpp_" #symbol, symbol)) { \
        SetText(detail, cap, L"Thiếu IL2CPP export cho AutoFight service bridge"); \
        return false; \
    } \
} while (0)
        ACTION_NEED(get_corlib);
        ACTION_NEED(class_get_name);
        ACTION_NEED(object_get_class);
        ACTION_NEED(value_box);
        ACTION_NEED(gchandle_new);
        ACTION_NEED(gchandle_get_target);
        ACTION_NEED(gchandle_free);
#undef ACTION_NEED
        return true;
    }
};

ActionApi g_actionApi;

bool ResolveCore(Il2CppClass*& gameApi, Il2CppClass*& session, Il2CppClass*& guiApi,
                 wchar_t* detail, std::size_t cap) {
    gameApi = nullptr;
    session = nullptr;
    guiApi = nullptr;
    if (!g_api.Load(detail, cap) || !g_actionApi.Load(detail, cap)) return false;
    const Il2CppImage* image = GameImage();
    if (!image) {
        SetText(detail, cap, L"Không mở được Assembly-CSharp");
        return false;
    }
    gameApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    session = g_api.class_from_name(image, "FGStudio.Game.Logic", "SessionData");
    guiApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_GUI");
    if (!gameApi || !session || !guiApi) {
        SetText(detail, cap, L"Thiếu class Game/Session/LuaSystemAPI_GUI");
        return false;
    }
    return true;
}

bool SafeForAction(Il2CppClass* gameApi, Il2CppClass* session,
                   wchar_t* detail, std::size_t cap) {
    const MethodInfo* mapReady = FindMethod(gameApi, "IsMapReady", 0);
    const MethodInfo* waiting = FindMethod(session, "get_WaitingChangeMap", 0);
    if (!mapReady || !waiting || !StaticMethod(mapReady) || !StaticMethod(waiting)) {
        SetText(detail, cap, L"Không resolve được guard chuyển map");
        return false;
    }
    bool ready = false;
    bool changing = false;
    if (!InvokeBool(mapReady, nullptr, ready, detail, cap)) return false;
    if (!InvokeBool(waiting, nullptr, changing, detail, cap)) return false;
    if (!ready || changing) {
        SetText(detail, cap, L"Action bị chặn: map chưa ready/đang chuyển map");
        return false;
    }
    return true;
}

bool InvokeInt32(const MethodInfo* method, void* instance, std::int32_t& out,
                 wchar_t* detail, std::size_t cap) {
    out = 0;
    if (!method) {
        SetText(detail, cap, L"Int32 getter chưa resolve");
        return false;
    }
    const Il2CppType* returnType = g_api.method_get_return_type(method);
    char* typeName = returnType ? g_api.type_get_name(returnType) : nullptr;
    if (!typeName || !Eq(typeName, "System.Int32")) {
        if (typeName) g_api.free_fn(typeName);
        SetText(detail, cap, L"Int32 getter có return type không đúng");
        return false;
    }
    g_api.free_fn(typeName);
    void* exception = nullptr;
    Il2CppObject* boxed = g_api.runtime_invoke(method, instance, nullptr, &exception);
    if (exception || !boxed) {
        SetText(detail, cap, L"Int32 getter ném exception/null");
        return false;
    }
    void* raw = g_api.object_unbox(boxed);
    if (!raw) {
        SetText(detail, cap, L"Không unbox được Int32");
        return false;
    }
    out = *reinterpret_cast<const std::int32_t*>(raw);
    return true;
}

bool InvokeObjectNoArgs(const MethodInfo* method, void* instance, Il2CppObject*& out,
                        wchar_t* detail, std::size_t cap) {
    out = nullptr;
    if (!method) {
        SetText(detail, cap, L"Object getter chưa resolve");
        return false;
    }
    void* exception = nullptr;
    out = g_api.runtime_invoke(method, instance, nullptr, &exception);
    if (exception || !out) {
        SetText(detail, cap, L"Object getter ném exception/null");
        out = nullptr;
        return false;
    }
    return true;
}

bool ProveUnityMainThread(wchar_t* detail, std::size_t cap) {
    const Il2CppImage* core = g_actionApi.get_corlib();
    if (!core) {
        SetText(detail, cap, L"Không lấy được corlib để proof main thread");
        return false;
    }
    Il2CppClass* sync = g_api.class_from_name(core, "System.Threading", "SynchronizationContext");
    Il2CppClass* threadClass = g_api.class_from_name(core, "System.Threading", "Thread");
    if (!sync || !threadClass) {
        SetText(detail, cap, L"Thiếu SynchronizationContext/Thread cho proof main thread");
        return false;
    }
    const MethodInfo* getCurrentContext = FindMethod(sync, "get_Current", 0);
    const MethodInfo* getCurrentThread = FindMethod(threadClass, "get_CurrentThread", 0);
    if (!getCurrentContext || !getCurrentThread ||
        !StaticMethod(getCurrentContext) || !StaticMethod(getCurrentThread)) {
        SetText(detail, cap, L"Không resolve được current context/thread");
        return false;
    }
    Il2CppObject* context = nullptr;
    Il2CppObject* thread = nullptr;
    if (!InvokeObjectNoArgs(getCurrentContext, nullptr, context, detail, cap) ||
        !InvokeObjectNoArgs(getCurrentThread, nullptr, thread, detail, cap)) return false;

    Il2CppClass* contextClass = g_actionApi.object_get_class(context);
    const char* contextName = contextClass ? g_actionApi.class_get_name(contextClass) : nullptr;
    if (!contextName || !Eq(contextName, "UnitySynchronizationContext")) {
        SetText(detail, cap, L"Callback không nằm trong UnitySynchronizationContext");
        return false;
    }
    const MethodInfo* getMainThreadId = FindMethod(contextClass, "get_MainThreadId", 0);
    const MethodInfo* getManagedThreadId = FindMethod(threadClass, "get_ManagedThreadId", 0);
    std::int32_t mainId = 0;
    std::int32_t currentId = 0;
    if (!InvokeInt32(getMainThreadId, context, mainId, detail, cap) ||
        !InvokeInt32(getManagedThreadId, thread, currentId, detail, cap)) return false;
    if (mainId != currentId) {
        SetText(detail, cap, L"Managed thread ID không phải Unity main thread");
        return false;
    }
    return true;
}

bool MatchBytes(const std::uint8_t* address, const std::uint8_t* expected, std::size_t count) {
    if (!address || !expected) return false;
    for (std::size_t i = 0; i < count; ++i) {
        if (address[i] != expected[i]) return false;
    }
    return true;
}

bool ValidateFrozenExecutorDonor(wchar_t* detail, std::size_t cap) {
    if (!g_api.module) {
        SetText(detail, cap, L"GameAssembly chưa resolve cho donor validation");
        return false;
    }
    const auto* base = reinterpret_cast<const std::uint8_t*>(g_api.module);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        SetText(detail, cap, L"GameAssembly không có DOS header hợp lệ");
        return false;
    }
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->FileHeader.TimeDateStamp != 0x6A410C14u ||
        nt->OptionalHeader.SizeOfImage != 0x03DCB000u) {
        SetText(detail, cap, L"Client khác frozen donor; chặn RVA ExecuteUiObject");
        return false;
    }
    constexpr std::uintptr_t GetExecutorRva = 0x523CE0u;
    constexpr std::uintptr_t ExecuteUiObjectRva = 0x521B20u;
    const std::uint8_t getSig[] = {0x48,0x83,0xEC,0x28,0x80,0x3D,0x7C,0x38,0x2A,0x03,0x00,0x75};
    const std::uint8_t execSig[] = {0x40,0x53,0x55,0x56,0x57,0x48,0x83,0xEC,0x38,0x80,0x3D,0x4D};
    if (!MatchBytes(base + GetExecutorRva, getSig, sizeof(getSig)) ||
        !MatchBytes(base + ExecuteUiObjectRva, execSig, sizeof(execSig))) {
        SetText(detail, cap, L"Byte signature donor ExecuteUiObject không khớp; action bị chặn");
        return false;
    }
    return true;
}

struct RootHandle {
    std::uint32_t value = 0;
    ~RootHandle() {
        if (value && g_actionApi.gchandle_free) g_actionApi.gchandle_free(value);
    }
    bool Hold(Il2CppObject* object) {
        if (!object) return false;
        value = g_actionApi.gchandle_new(object, false);
        return value != 0;
    }
};

bool FindLuaUi(Il2CppClass* guiApi, const char* uiName, Il2CppObject*& ui,
               wchar_t* detail, std::size_t cap) {
    ui = nullptr;
    const char* oneString[] = {"System.String"};
    const MethodInfo* find = FindMethodExact(guiApi, "FindUI", oneString, 1);
    if (!find) find = FindMethodExact(guiApi, "MainFindUI", oneString, 1);
    if (!find || !StaticMethod(find)) {
        SetText(detail, cap, L"Không resolve được LuaSystemAPI_GUI.FindUI/MainFindUI(String)");
        return false;
    }
    Il2CppString* name = g_api.string_new(uiName);
    if (!name) {
        SetText(detail, cap, L"Không tạo được tên Lua UI");
        return false;
    }
    RootHandle nameRoot;
    if (!nameRoot.Hold(reinterpret_cast<Il2CppObject*>(name))) {
        SetText(detail, cap, L"Không root được tên Lua UI");
        return false;
    }
    void* args[] = {&name};
    void* exception = nullptr;
    ui = g_api.runtime_invoke(find, nullptr, args, &exception);
    if (exception || !ui) {
        SetText(detail, cap, L"Không tìm thấy AutoFight_Main bằng FindUI/MainFindUI");
        ui = nullptr;
        return false;
    }
    return true;
}

bool ReadEnableAutoF1(Il2CppClass* gameApi, bool& value) {
    wchar_t ignored[128]{};
    const MethodInfo* getter = FindMethod(gameApi, "get_EnableAutoF1", 0);
    return getter && StaticMethod(getter) &&
           InvokeBool(getter, nullptr, value, ignored, _countof(ignored));
}

bool ExecuteAutoFightMain(Il2CppClass* gameApi, Il2CppClass* guiApi, std::int32_t mode,
                          wchar_t* detail, std::size_t cap) {
    if (!ProveUnityMainThread(detail, cap)) return false;
    if (!ValidateFrozenExecutorDonor(detail, cap)) return false;

    Il2CppObject* autoFightMain = nullptr;
    if (!FindLuaUi(guiApi, "AutoFight_Main", autoFightMain, detail, cap)) return false;

    const Il2CppImage* core = g_actionApi.get_corlib();
    Il2CppClass* objectClass = core ? g_api.class_from_name(core, "System", "Object") : nullptr;
    Il2CppClass* int32Class = core ? g_api.class_from_name(core, "System", "Int32") : nullptr;
    if (!objectClass || !int32Class) {
        SetText(detail, cap, L"Không resolve được System.Object/System.Int32");
        return false;
    }

    Il2CppObject* boxedMode = g_actionApi.value_box(int32Class, &mode);
    Il2CppArray* args = g_api.array_new(objectClass, 1);
    Il2CppString* functionName = g_api.string_new("StartAutoFight");
    if (!boxedMode || !args || !functionName) {
        SetText(detail, cap, L"Không tạo được boxed mode/Object[]/StartAutoFight string");
        return false;
    }

    RootHandle modeRoot;
    RootHandle argsRoot;
    RootHandle functionRoot;
    if (!modeRoot.Hold(boxedMode) ||
        !argsRoot.Hold(reinterpret_cast<Il2CppObject*>(args)) ||
        !functionRoot.Hold(reinterpret_cast<Il2CppObject*>(functionName))) {
        SetText(detail, cap, L"Không root được tham số Lua action");
        return false;
    }

    // Exact frozen-client x64 array payload offset used by the v0.9.0 donor.
    constexpr std::uintptr_t ArrayDataOffset = 0x20u;
    auto** first = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<std::uint8_t*>(args) + ArrayDataOffset);
    *first = boxedMode;
    MemoryBarrier();

    constexpr std::uintptr_t GetExecutorRva = 0x523CE0u;
    constexpr std::uintptr_t ExecuteUiObjectRva = 0x521B20u;
    using GetExecutorFn = Il2CppObject* (__fastcall*)(const MethodInfo*);
    using ExecuteUiObjectFn = Il2CppObject* (__fastcall*)(Il2CppObject*, Il2CppObject*,
                                                           Il2CppString*, Il2CppArray*,
                                                           const MethodInfo*);
    auto* base = reinterpret_cast<std::uint8_t*>(g_api.module);
    auto getExecutor = reinterpret_cast<GetExecutorFn>(base + GetExecutorRva);
    auto executeUiObject = reinterpret_cast<ExecuteUiObjectFn>(base + ExecuteUiObjectRva);
    Il2CppObject* executor = getExecutor(nullptr);
    if (!executor) {
        SetText(detail, cap, L"MonoBehaviourExecutor.Instance null");
        return false;
    }

    (void)executeUiObject(executor, autoFightMain, functionName, args, nullptr);

    bool enableAutoF1 = false;
    const bool stateReadable = ReadEnableAutoF1(gameApi, enableAutoF1);
    if (mode == 1) {
        SetText(detail, cap, L"Đã gọi AutoFight_Main.StartAutoFight(Train=1) trên Unity main thread");
        if (stateReadable) Append(detail, cap, enableAutoF1 ? L"; immediate EnableAutoF1=ON" : L"; immediate EnableAutoF1=OFF");
    } else {
        SetText(detail, cap, L"Đã gọi AutoFight_Main.StartAutoFight(None=0) trên Unity main thread");
        if (stateReadable) Append(detail, cap, enableAutoF1 ? L"; immediate EnableAutoF1=ON" : L"; immediate EnableAutoF1=OFF");
    }
    Append(detail, cap, L"; cần xác nhận hành vi thực tế trong game");
    return true;
}
