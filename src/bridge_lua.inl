bool ResolveCore(Il2CppClass*& gameApi, Il2CppClass*& session, Il2CppClass*& luaManager,
                 wchar_t* detail, std::size_t cap) {
    gameApi = nullptr;
    session = nullptr;
    luaManager = nullptr;
    if (!g_api.Load(detail, cap)) return false;
    const Il2CppImage* image = GameImage();
    if (!image) {
        SetText(detail, cap, L"Không mở được Assembly-CSharp");
        return false;
    }
    gameApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    session = g_api.class_from_name(image, "FGStudio.Game.Logic", "SessionData");
    luaManager = g_api.class_from_name(image, "FGStudio.LuaSystem", "LuaSystemManager");
    if (!gameApi || !session || !luaManager) {
        SetText(detail, cap, L"Thiếu class Game/Session/LuaSystemManager");
        return false;
    }
    return true;
}

bool SafeForAction(Il2CppClass* gameApi, Il2CppClass* session, wchar_t* detail, std::size_t cap) {
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

bool FindStaticObjectField(Il2CppClass* klass, const char* name, Il2CppObject*& out) {
    out = nullptr;
    constexpr std::uint32_t StaticFieldFlag = 0x0010;
    for (Il2CppClass* c = klass; c; c = g_api.class_get_parent(c)) {
        FieldInfo* field = g_api.class_get_field_from_name(c, name);
        if (!field) continue;
        if (g_api.field_get_flags && (g_api.field_get_flags(field) & StaticFieldFlag) == 0) continue;
        g_api.field_static_get_value(field, &out);
        if (out) return true;
    }
    return false;
}

bool LuaManagerInstance(Il2CppClass* manager, Il2CppObject*& instance, wchar_t* detail, std::size_t cap) {
    instance = nullptr;
    const char* getters[] = {"get_Instance", "get_instance"};
    for (const char* getter : getters) {
        const MethodInfo* method = FindMethod(manager, getter, 0);
        if (!method || !StaticMethod(method)) continue;
        void* exception = nullptr;
        instance = g_api.runtime_invoke(method, nullptr, nullptr, &exception);
        if (!exception && instance) return true;
    }
    const char* fields[] = {"Instance", "instance", "_instance", "m_Instance", "<Instance>k__BackingField"};
    for (const char* field : fields) {
        if (FindStaticObjectField(manager, field, instance)) return true;
    }
    SetText(detail, cap, L"Không lấy được LuaSystemManager instance");
    return false;
}

bool ExecuteLuaNoArg(Il2CppClass* manager, const char* scriptName, const char* functionName,
                     wchar_t* detail, std::size_t cap) {
    const char* exec2Types[] = {"System.String", "System.String"};
    const char* exec3Types[] = {"System.String", "System.String", "System.Object[]"};

    const MethodInfo* exec2 = FindMethodExact(manager, "ExecuteFunction", exec2Types, 2);
    const MethodInfo* exec3 = FindMethodExact(manager, "ExecuteFunction", exec3Types, 3);
    const MethodInfo* exec = exec3 ? exec3 : exec2;
    if (!exec) {
        AppendExecuteFunctionSignatures(manager, detail, cap);
        return false;
    }

    Il2CppObject* instance = nullptr;
    if (!StaticMethod(exec) && !LuaManagerInstance(manager, instance, detail, cap)) return false;

    Il2CppString* script = g_api.string_new(scriptName);
    Il2CppString* function = g_api.string_new(functionName);
    if (!script || !function) {
        SetText(detail, cap, L"Không tạo được managed Lua strings");
        return false;
    }

    void* exception = nullptr;
    if (exec3) {
        const Il2CppImage* core = AssemblyImage("mscorlib", "mscorlib.dll");
        Il2CppClass* objectClass = core ? g_api.class_from_name(core, "System", "Object") : nullptr;
        if (!objectClass) {
            SetText(detail, cap, L"Không resolve được System.Object");
            return false;
        }
        Il2CppArray* emptyArgs = g_api.array_new(objectClass, 0);
        if (!emptyArgs) {
            SetText(detail, cap, L"Không tạo được Object[] rỗng");
            return false;
        }
        void* args[] = {&script, &function, &emptyArgs};
        (void)g_api.runtime_invoke(exec, StaticMethod(exec) ? nullptr : instance, args, &exception);
    } else {
        void* args[] = {&script, &function};
        (void)g_api.runtime_invoke(exec, StaticMethod(exec) ? nullptr : instance, args, &exception);
    }

    if (exception) {
        SetText(detail, cap, L"Lua ExecuteFunction ném managed/Lua exception");
        return false;
    }

    SetText(detail, cap, exec3 ? L"ExecuteFunction exact 3-param matched; "
                               : L"ExecuteFunction exact 2-param matched; ");
    Append(detail, cap, L"Lua wrapper returned: TopIcon.");
    Append(detail, cap, functionName[4] == 'T' ? L"AutoTrainClick" : L"AutoStopClick");
    Append(detail, cap, L"; runtime effect phải xác nhận trong game");
    return true;
}
