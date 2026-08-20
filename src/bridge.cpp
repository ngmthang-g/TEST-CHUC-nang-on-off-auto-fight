#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <climits>
#include <cstring>
#include "protocol.h"

using namespace cleanroute;

namespace {

using Il2CppDomain = void;
using Il2CppAssembly = void;
using Il2CppImage = void;
using Il2CppClass = void;
using MethodInfo = void;
using FieldInfo = void;
using Il2CppType = void;
using Il2CppObject = void;
using Il2CppString = void;
using Il2CppArray = void;

HANDLE g_mapping = nullptr;
SharedBlock* g_shared = nullptr;

template <typename T>
bool Resolve(HMODULE module, const char* name, T& out) {
    out = nullptr;
    FARPROC p = GetProcAddress(module, name);
    if (!p) return false;
    static_assert(sizeof(p) == sizeof(out), "pointer-size mismatch");
    const unsigned char* s = reinterpret_cast<const unsigned char*>(&p);
    unsigned char* d = reinterpret_cast<unsigned char*>(&out);
    for (std::size_t i = 0; i < sizeof(out); ++i) d[i] = s[i];
    return out != nullptr;
}

bool Eq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) { if (*a++ != *b++) return false; }
    return *a == *b;
}

void SetText(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || cap == 0) return;
    std::size_t i = 0;
    if (text) while (i + 1 < cap && text[i]) { out[i] = text[i]; ++i; }
    out[i] = 0;
}

void Append(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || !text || cap == 0) return;
    std::size_t n = 0; while (n + 1 < cap && out[n]) ++n;
    std::size_t i = 0; while (n + 1 < cap && text[i]) out[n++] = text[i++];
    out[n] = 0;
}

void AppendInt(wchar_t* out, std::size_t cap, int value) {
    wchar_t tmp[32]{}; wsprintfW(tmp, L"%d", value); Append(out, cap, tmp);
}

struct Api {
    HMODULE module = nullptr;
    Il2CppDomain* (__cdecl* domain_get)() = nullptr;
    const Il2CppAssembly* (__cdecl* domain_assembly_open)(Il2CppDomain*, const char*) = nullptr;
    const Il2CppImage* (__cdecl* assembly_get_image)(const Il2CppAssembly*) = nullptr;
    Il2CppClass* (__cdecl* class_from_name)(const Il2CppImage*, const char*, const char*) = nullptr;
    const MethodInfo* (__cdecl* class_get_method_from_name)(Il2CppClass*, const char*, int) = nullptr;
    const MethodInfo* (__cdecl* class_get_methods)(Il2CppClass*, void**) = nullptr;
    const char* (__cdecl* method_get_name)(const MethodInfo*) = nullptr;
    const char* (__cdecl* class_get_name)(Il2CppClass*) = nullptr;
    Il2CppClass* (__cdecl* class_get_parent)(Il2CppClass*) = nullptr;
    std::uint32_t (__cdecl* method_get_flags)(const MethodInfo*, std::uint32_t*) = nullptr;
    std::uint32_t (__cdecl* method_get_param_count)(const MethodInfo*) = nullptr;
    const Il2CppType* (__cdecl* method_get_param)(const MethodInfo*, std::uint32_t) = nullptr;
    const Il2CppType* (__cdecl* method_get_return_type)(const MethodInfo*) = nullptr;
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, void**) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    FieldInfo* (__cdecl* class_get_field_from_name)(Il2CppClass*, const char*) = nullptr;
    const Il2CppType* (__cdecl* field_get_type)(FieldInfo*) = nullptr;
    void (__cdecl* field_get_value)(Il2CppObject*, FieldInfo*, void*) = nullptr;
    Il2CppClass* (__cdecl* class_from_type)(const Il2CppType*) = nullptr;
    bool (__cdecl* class_is_valuetype)(const Il2CppClass*) = nullptr;
    std::int32_t (__cdecl* string_length)(Il2CppString*) = nullptr;
    const wchar_t* (__cdecl* string_chars)(Il2CppString*) = nullptr;
    Il2CppString* (__cdecl* string_new)(const char*) = nullptr;
    Il2CppArray* (__cdecl* array_new)(Il2CppClass*, std::uintptr_t) = nullptr;
    const Il2CppImage* (__cdecl* get_corlib)() = nullptr;
    Il2CppObject* (__cdecl* value_box)(Il2CppClass*, void*) = nullptr;
    std::uint32_t (__cdecl* gchandle_new)(Il2CppObject*, bool) = nullptr;
    Il2CppObject* (__cdecl* gchandle_get_target)(std::uint32_t) = nullptr;
    void (__cdecl* gchandle_free)(std::uint32_t) = nullptr;

    bool Load(wchar_t* detail, std::size_t cap) {
        if (module) return true;
        module = GetModuleHandleW(L"GameAssembly.dll");
        if (!module) { SetText(detail, cap, L"GameAssembly.dll chưa sẵn sàng"); return false; }
#define NEED(symbol) do { if (!Resolve(module, "il2cpp_" #symbol, symbol)) { SetText(detail, cap, L"Thiếu IL2CPP export bắt buộc"); return false; } } while (0)
        NEED(domain_get); NEED(domain_assembly_open); NEED(assembly_get_image); NEED(class_from_name);
        NEED(class_get_method_from_name); NEED(class_get_parent); NEED(method_get_flags);
        NEED(method_get_param_count); NEED(method_get_param); NEED(method_get_return_type);
        NEED(type_get_name); NEED(runtime_invoke); NEED(object_unbox); NEED(object_get_class);
        NEED(class_get_field_from_name); NEED(field_get_type); NEED(field_get_value);
        NEED(class_from_type); NEED(class_is_valuetype); NEED(string_length); NEED(string_chars);
#undef NEED
        if (!Resolve(module, "il2cpp_free", free_fn)) { SetText(detail, cap, L"Thiếu il2cpp_free"); return false; }
        return true;
    }
};

Api g_api;

bool LoadDungeonExports(bool action, wchar_t* detail, std::size_t cap) {
    if (!g_api.Load(detail, cap)) return false;
#define EXT_NEED(symbol) do { \
    if (!g_api.symbol && !Resolve(g_api.module, "il2cpp_" #symbol, g_api.symbol)) { \
        SetText(detail, cap, L"Thiếu IL2CPP export mở rộng cho AUTO PHÓ BẢN"); \
        return false; \
    } \
} while (0)
    EXT_NEED(class_get_methods);
    EXT_NEED(method_get_name);
    EXT_NEED(class_get_name);
    if (action) {
        EXT_NEED(string_new);
        EXT_NEED(array_new);
        EXT_NEED(get_corlib);
        EXT_NEED(value_box);
        EXT_NEED(gchandle_new);
        EXT_NEED(gchandle_get_target);
        EXT_NEED(gchandle_free);
    }
#undef EXT_NEED
    return true;
}

const Il2CppImage* Image() {
    Il2CppDomain* domain = g_api.domain_get ? g_api.domain_get() : nullptr;
    if (!domain) return nullptr;
    const Il2CppAssembly* assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp");
    if (!assembly) assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp.dll");
    return assembly ? g_api.assembly_get_image(assembly) : nullptr;
}

bool StaticMethod(const MethodInfo* method) {
    if (!method) return false;
    constexpr std::uint32_t StaticFlag = 0x0010;
    std::uint32_t iflags = 0;
    return (g_api.method_get_flags(method, &iflags) & StaticFlag) != 0;
}

const MethodInfo* FindMethod(Il2CppClass* klass, const char* name, int argc) {
    for (Il2CppClass* c = klass; c; c = g_api.class_get_parent(c)) {
        if (const MethodInfo* m = g_api.class_get_method_from_name(c, name, argc)) return m;
    }
    return nullptr;
}

bool ParamType(const MethodInfo* m, std::uint32_t index, const char* expected) {
    if (!m || index >= g_api.method_get_param_count(m)) return false;
    const Il2CppType* t = g_api.method_get_param(m, index);
    char* n = t ? g_api.type_get_name(t) : nullptr;
    if (!n) return false;
    bool ok = Eq(n, expected);
    g_api.free_fn(n);
    return ok;
}

const MethodInfo* ExactMethod(Il2CppClass* klass, const char* name, int argc, bool isStatic,
                              const char* p0 = nullptr, const char* p1 = nullptr, const char* p2 = nullptr) {
    const MethodInfo* m = FindMethod(klass, name, argc);
    if (!m || StaticMethod(m) != isStatic) return nullptr;
    if (argc > 0 && p0 && !ParamType(m, 0, p0)) return nullptr;
    if (argc > 1 && p1 && !ParamType(m, 1, p1)) return nullptr;
    if (argc > 2 && p2 && !ParamType(m, 2, p2)) return nullptr;
    return m;
}

const MethodInfo* FindMethodExact(Il2CppClass* klass, const char* name,
                                  const char* const* paramTypes, std::uint32_t count) {
    if (!klass || !name || !g_api.class_get_methods || !g_api.method_get_name) return nullptr;
    for (Il2CppClass* c = klass; c; c = g_api.class_get_parent(c)) {
        void* iter = nullptr;
        while (const MethodInfo* method = g_api.class_get_methods(c, &iter)) {
            if (!Eq(g_api.method_get_name(method), name) ||
                g_api.method_get_param_count(method) != count) continue;
            bool match = true;
            for (std::uint32_t i = 0; i < count; ++i) {
                if (!ParamType(method, i, paramTypes[i])) { match = false; break; }
            }
            if (match) return method;
        }
    }
    return nullptr;
}

bool ClassChainHasExactName(Il2CppClass* klass, const char* expected) {
    if (!klass || !expected || !*expected || !g_api.class_get_name || !g_api.class_get_parent) return false;
    // Deliberately exact. Substring tests such as "Role"/"Monster" mixed player
    // and monster families and are forbidden for the dungeon counter.
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        if (Eq(g_api.class_get_name(current), expected)) return true;
    }
    return false;
}

void CopyAscii(const char* text, wchar_t* out, std::size_t cap) {
    if (!out || cap == 0) return;
    std::size_t i = 0;
    if (text) {
        while (i + 1 < cap && text[i]) {
            out[i] = static_cast<unsigned char>(text[i]);
            ++i;
        }
    }
    out[i] = 0;
}

bool InvokeObjectArgs(const MethodInfo* method, void* instance, void** args,
                      Il2CppObject*& out, wchar_t* detail, std::size_t cap) {
    out = nullptr;
    if (!method) { SetText(detail, cap, L"Method object chưa resolve"); return false; }
    void* exc = nullptr;
    out = g_api.runtime_invoke(method, instance, args, &exc);
    if (exc) { SetText(detail, cap, L"Managed exception ở object getter"); return false; }
    return true;
}

bool InvokeObject(const MethodInfo* method, void* instance, Il2CppObject*& out, wchar_t* detail, std::size_t cap) {
    return InvokeObjectArgs(method, instance, nullptr, out, detail, cap);
}

bool InvokeScalar(const MethodInfo* method, void* instance, std::int64_t& out, wchar_t* detail, std::size_t cap) {
    out = 0;
    if (!method) { SetText(detail, cap, L"Scalar method chưa resolve"); return false; }
    const Il2CppType* rt = g_api.method_get_return_type(method);
    char* tn = rt ? g_api.type_get_name(rt) : nullptr;
    if (!tn) { SetText(detail, cap, L"Không đọc được return type"); return false; }
    void* exc = nullptr;
    Il2CppObject* boxed = g_api.runtime_invoke(method, instance, nullptr, &exc);
    if (exc || !boxed) { g_api.free_fn(tn); SetText(detail, cap, L"Scalar getter lỗi/null"); return false; }
    void* raw = g_api.object_unbox(boxed);
    if (!raw) { g_api.free_fn(tn); SetText(detail, cap, L"Không unbox scalar"); return false; }
    bool ok = true;
    if (Eq(tn, "System.Boolean")) out = *reinterpret_cast<const std::uint8_t*>(raw) ? 1 : 0;
    else if (Eq(tn, "System.Int32")) out = *reinterpret_cast<const std::int32_t*>(raw);
    else if (Eq(tn, "System.UInt32")) out = *reinterpret_cast<const std::uint32_t*>(raw);
    else if (Eq(tn, "System.Int64")) out = *reinterpret_cast<const std::int64_t*>(raw);
    else ok = false;
    g_api.free_fn(tn);
    if (!ok) SetText(detail, cap, L"Return type scalar chưa hỗ trợ");
    return ok;
}

bool ScalarGetter(Il2CppClass* klass, const char* name, void* instance, std::int32_t& out,
                  wchar_t* detail, std::size_t cap) {
    std::int64_t value = 0;
    if (!InvokeScalar(FindMethod(klass, name, 0), instance, value, detail, cap)) return false;
    if (value < INT32_MIN || value > INT32_MAX) { SetText(detail, cap, L"Scalar vượt Int32"); return false; }
    out = static_cast<std::int32_t>(value);
    return true;
}

bool TryScalarGetter(Il2CppClass* klass, void* instance, std::int32_t& out,
                     const char* a, const char* b = nullptr, const char* c = nullptr) {
    const char* names[] = {a, b, c};
    for (const char* name : names) {
        if (!name) continue;
        wchar_t ignored[96]{};
        if (ScalarGetter(klass, name, instance, out, ignored, _countof(ignored))) return true;
    }
    return false;
}

bool TryIntField(Il2CppClass* klass, Il2CppObject* instance, std::int32_t& out,
                 const char* a, const char* b = nullptr, const char* c = nullptr,
                 const char* d = nullptr) {
    const char* names[] = {a, b, c, d};
    for (const char* name : names) {
        if (!name) continue;
        FieldInfo* field = nullptr;
        for (Il2CppClass* current = klass; current && !field; current = g_api.class_get_parent(current))
            field = g_api.class_get_field_from_name(current, name);
        if (!field) continue;
        const Il2CppType* type = g_api.field_get_type(field);
        char* typeName = type ? g_api.type_get_name(type) : nullptr;
        const bool int32 = typeName && (Eq(typeName, "System.Int32") || Eq(typeName, "System.UInt32"));
        if (typeName) g_api.free_fn(typeName);
        if (!int32) continue;
        g_api.field_get_value(instance, field, &out);
        return true;
    }
    return false;
}

bool CopyString(Il2CppString* value, wchar_t* out, std::size_t cap);

bool TryStringGetter(Il2CppClass* klass, void* instance, wchar_t* out, std::size_t cap,
                     const char* a, const char* b = nullptr) {
    const char* names[] = {a, b};
    for (const char* name : names) {
        if (!name) continue;
        const MethodInfo* method = FindMethod(klass, name, 0);
        if (!method) continue;
        Il2CppObject* object = nullptr;
        wchar_t ignored[96]{};
        if (InvokeObject(method, instance, object, ignored, _countof(ignored)) && object &&
            CopyString(reinterpret_cast<Il2CppString*>(object), out, cap)) return true;
    }
    return false;
}

bool StaticScalar(Il2CppClass* klass, const char* name, std::int32_t& out,
                  wchar_t* detail, std::size_t cap) {
    const MethodInfo* m = FindMethod(klass, name, 0);
    if (!m || !StaticMethod(m)) { SetText(detail, cap, L"Static getter chưa resolve"); return false; }
    return ScalarGetter(klass, name, nullptr, out, detail, cap);
}

bool InvokeVoid(const MethodInfo* method, void* instance, void** args,
                wchar_t* detail, std::size_t cap) {
    if (!method) { SetText(detail, cap, L"Action method chưa resolve"); return false; }
    void* exc = nullptr;
    (void)g_api.runtime_invoke(method, instance, args, &exc);
    if (exc) { SetText(detail, cap, L"Action ném managed exception"); return false; }
    return true;
}

bool CopyString(Il2CppString* value, wchar_t* out, std::size_t cap) {
    if (!value || !out || cap == 0) return false;
    const int len = g_api.string_length(value);
    const wchar_t* chars = g_api.string_chars(value);
    if (len < 0 || len > 4096 || !chars) return false;
    std::size_t n = static_cast<std::size_t>(len);
    if (n + 1 > cap) n = cap - 1;
    for (std::size_t i = 0; i < n; ++i) out[i] = chars[i];
    out[n] = 0;
    return true;
}

struct Classes {
    Il2CppClass* gameApi = nullptr;
    Il2CppClass* guiApi = nullptr; // optional observer surface; route core must remain usable if unavailable
    Il2CppClass* session = nullptr;
    Il2CppClass* shared = nullptr;
    Il2CppClass* autoPath = nullptr;
};

bool ResolveClasses(Classes& c, wchar_t* detail, std::size_t cap) {
    if (!g_api.Load(detail, cap)) return false;
    const Il2CppImage* image = Image();
    if (!image) { SetText(detail, cap, L"Không mở được Assembly-CSharp"); return false; }
    c.gameApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    c.guiApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_GUI");
    c.session = g_api.class_from_name(image, "FGStudio.Game.Logic", "SessionData");
    c.shared = g_api.class_from_name(image, "FGStudio.LuaSystem", "LuaSystemSharedData");
    c.autoPath = g_api.class_from_name(image, "FGStudio.Engine.Logic", "AutoPathManager");
    if (!c.gameApi || !c.session || !c.shared || !c.autoPath) {
        SetText(detail, cap, L"Thiếu class route bắt buộc trên client này");
        return false;
    }
    return true;
}

bool Transition(const Classes& c, int& mapReady, int& waiting, wchar_t* detail, std::size_t cap) {
    if (!StaticScalar(c.gameApi, "IsMapReady", mapReady, detail, cap)) return false;
    if (!StaticScalar(c.session, "get_WaitingChangeMap", waiting, detail, cap)) return false;
    return true;
}

bool GetLeader(const Classes& c, Il2CppObject*& leader, Il2CppClass*& leaderClass,
               wchar_t* detail, std::size_t cap) {
    const MethodInfo* getLeader = ExactMethod(c.shared, "get_LeaderRoleData", 0, true);
    if (!getLeader || !InvokeObject(getLeader, nullptr, leader, detail, cap) || !leader) {
        SetText(detail, cap, L"LeaderRoleData chưa sẵn sàng"); return false;
    }
    leaderClass = g_api.object_get_class(leader);
    if (!leaderClass) { SetText(detail, cap, L"Không lấy được class LeaderRoleData"); return false; }
    return true;
}

bool ReadPosition(Il2CppObject* leader, Il2CppClass* leaderClass, int& x, int& y,
                  wchar_t* detail, std::size_t cap) {
    if (ScalarGetter(leaderClass, "get_PosX", leader, x, detail, cap) &&
        ScalarGetter(leaderClass, "get_PosY", leader, y, detail, cap)) return true;
    FieldInfo* field = nullptr;
    for (Il2CppClass* c = leaderClass; c; c = g_api.class_get_parent(c)) {
        field = g_api.class_get_field_from_name(c, "roleData");
        if (field) break;
    }
    if (!field) { SetText(detail, cap, L"Không resolve được PosX/PosY"); return false; }
    const Il2CppType* ft = g_api.field_get_type(field);
    Il2CppClass* fc = ft ? g_api.class_from_type(ft) : nullptr;
    if (!fc || g_api.class_is_valuetype(fc)) { SetText(detail, cap, L"roleData backing không hợp lệ"); return false; }
    Il2CppObject* backing = nullptr;
    g_api.field_get_value(leader, field, &backing);
    if (!backing) { SetText(detail, cap, L"roleData backing=null"); return false; }
    Il2CppClass* bc = g_api.object_get_class(backing);
    return bc && ScalarGetter(bc, "get_PosX", backing, x, detail, cap) &&
                 ScalarGetter(bc, "get_PosY", backing, y, detail, cap);
}

bool AutoPathInstance(const Classes& c, Il2CppObject*& instance, Il2CppClass*& actual,
                      wchar_t* detail, std::size_t cap) {
    const MethodInfo* getInstance = ExactMethod(c.autoPath, "get_Instance", 0, true);
    if (!getInstance || !InvokeObject(getInstance, nullptr, instance, detail, cap) || !instance) {
        SetText(detail, cap, L"AutoPathManager.Instance chưa sẵn sàng"); return false;
    }
    actual = g_api.object_get_class(instance);
    if (!actual) { SetText(detail, cap, L"Không lấy được class AutoPathManager"); return false; }
    return true;
}

bool ReadState(Snapshot& s, wchar_t* detail, std::size_t cap) {
    Classes c{};
    if (!ResolveClasses(c, detail, cap)) return false;
    s = {};
    int ready = 0, waiting = 0;
    if (!Transition(c, ready, waiting, detail, cap)) return false;
    s.mapReady = ready ? 1 : 0;
    s.waitingChangeMap = waiting ? 1 : 0;
    s.validMask |= ValidMapTransition;
    if (!ready || waiting) {
        SetText(detail, cap, L"Đang chuyển map; không đọc object sâu và không gửi action");
        return true;
    }

    Il2CppObject* leader = nullptr; Il2CppClass* lc = nullptr;
    if (!GetLeader(c, leader, lc, detail, cap)) return false;
    int role = 0, map = 0, x = 0, y = 0, riding = 0;
    if (!ScalarGetter(lc, "get_RoleID", leader, role, detail, cap) || role <= 0) return false;
    if (!ScalarGetter(lc, "get_MapID", leader, map, detail, cap) || map <= 0) return false;
    if (!ReadPosition(leader, lc, x, y, detail, cap)) return false;
    if (!ScalarGetter(lc, "get_IsRiding", leader, riding, detail, cap)) return false;
    s.roleID = role; s.mapID = map; s.x = x; s.y = y; s.riding = riding ? 1 : 0;
    s.validMask |= ValidIdentity | ValidMap | ValidPosition | ValidRiding;

    int dead = 0;
    wchar_t optionalDetail[160]{};
    if (ScalarGetter(lc, "get_IsDeath", leader, dead, optionalDetail, _countof(optionalDetail))) {
        s.dead = dead ? 1 : 0; s.validMask |= ValidLifeState;
    }


    int enableAutoF1 = 0;
    optionalDetail[0] = 0;
    if (StaticScalar(c.gameApi, "get_EnableAutoF1", enableAutoF1, optionalDetail, _countof(optionalDetail))) {
        // Existing client semantic verified by the read-only NewCore donor: EnableAutoF1=false means auto-fight is ON.
        s.autoFight = enableAutoF1 ? 0 : 1; s.validMask |= ValidAutoFight;
    }

    int freeBagSpace = -1;
    optionalDetail[0] = 0;
    if (StaticScalar(c.gameApi, "GetFreeBagSpace", freeBagSpace, optionalDetail, _countof(optionalDetail)) && freeBagSpace >= 0) {
        s.freeBagSpace = freeBagSpace; s.validMask |= ValidBagSpace;
    }


    Il2CppObject* ap = nullptr; Il2CppClass* ac = nullptr;
    if (!AutoPathInstance(c, ap, ac, detail, cap)) return false;
    int pathing = 0;
    if (!ScalarGetter(ac, "get_IsAutoPathing", ap, pathing, detail, cap)) return false;
    s.autoPathing = pathing ? 1 : 0; s.validMask |= ValidAutoPath;

    const MethodInfo* getName = FindMethod(lc, "get_Name", 0);
    if (getName) {
        Il2CppObject* no = nullptr;
        wchar_t ignored[128]{};
        if (InvokeObject(getName, leader, no, ignored, _countof(ignored)) && no)
            (void)CopyString(reinterpret_cast<Il2CppString*>(no), s.characterName, _countof(s.characterName));
    }

    SetText(detail, cap, L"STATE map="); AppendInt(detail, cap, s.mapID);
    Append(detail, cap, L" pos="); AppendInt(detail, cap, s.x); Append(detail, cap, L","); AppendInt(detail, cap, s.y);
    Append(detail, cap, L" riding="); AppendInt(detail, cap, s.riding);
    Append(detail, cap, L" autoPath="); AppendInt(detail, cap, s.autoPathing);
    if (s.validMask & ValidLifeState) { Append(detail, cap, L" dead="); AppendInt(detail, cap, s.dead); }
    if (s.validMask & ValidAutoFight) { Append(detail, cap, L" autoFight="); AppendInt(detail, cap, s.autoFight); }
    if (s.validMask & ValidBagSpace) { Append(detail, cap, L" freeBag="); AppendInt(detail, cap, s.freeBagSpace); }
    return true;
}

bool SafeForAction(const Classes& c, wchar_t* detail, std::size_t cap) {
    int ready = 0, waiting = 0;
    if (!Transition(c, ready, waiting, detail, cap)) return false;
    if (!ready || waiting) { SetText(detail, cap, L"Action bị chặn: đang chuyển map"); return false; }
    return true;
}

bool ToggleRide(bool desiredRiding, wchar_t* detail, std::size_t cap) {
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    Il2CppObject* leader = nullptr; Il2CppClass* lc = nullptr;
    if (!GetLeader(c, leader, lc, detail, cap)) return false;
    int riding = 0;
    if (!ScalarGetter(lc, "get_IsRiding", leader, riding, detail, cap)) return false;
    if ((riding != 0) == desiredRiding) { SetText(detail, cap, L"Ride state đã đúng; không toggle lại"); return true; }

    const MethodInfo* getSlot = ExactMethod(c.gameApi, "get_CurrentMountSlot", 0, true);
    const MethodInfo* toggle = ExactMethod(c.gameApi, "SendToggleRideState", 1, true, "System.Int32");
    if (!getSlot || !toggle) { SetText(detail, cap, L"Không resolve được API lên/xuống ngựa"); return false; }
    std::int64_t slot64 = 0;
    if (!InvokeScalar(getSlot, nullptr, slot64, detail, cap) || slot64 < 0 || slot64 > INT32_MAX) return false;
    std::int32_t slot = static_cast<std::int32_t>(slot64);
    void* args[] = { &slot };
    if (!InvokeVoid(toggle, nullptr, args, detail, cap)) return false;
    SetText(detail, cap, desiredRiding ? L"Đã gửi lệnh lên ngựa" : L"Đã gửi lệnh xuống ngựa");
    return true;
}

bool StartPath(int mapID, int x, int y, wchar_t* detail, std::size_t cap) {
    if (mapID <= 0) { SetText(detail, cap, L"MapID đích không hợp lệ"); return false; }
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    Il2CppObject* ap = nullptr; Il2CppClass* ac = nullptr;
    if (!AutoPathInstance(c, ap, ac, detail, cap)) return false;
    const MethodInfo* start = ExactMethod(ac, "StartAutoPath", 3, false,
                                          "System.Int32", "System.Int32", "System.Int32");
    if (!start) { SetText(detail, cap, L"Không resolve đúng StartAutoPath(Int32,Int32,Int32)"); return false; }
    std::int32_t m = mapID, px = x, py = y;
    void* args[] = { &m, &px, &py };
    if (!InvokeVoid(start, ap, args, detail, cap)) return false;
    SetText(detail, cap, L"Đã gửi AutoPath tới map="); AppendInt(detail, cap, mapID);
    Append(detail, cap, L" x="); AppendInt(detail, cap, x); Append(detail, cap, L" y="); AppendInt(detail, cap, y);
    return true;
}

bool StopPath(wchar_t* detail, std::size_t cap) {
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    const MethodInfo* stop = ExactMethod(c.gameApi, "StopAutoPath", 0, true);
    if (!stop) { SetText(detail, cap, L"Không resolve được LuaSystemAPI_Game.StopAutoPath()"); return false; }
    if (!InvokeVoid(stop, nullptr, nullptr, detail, cap)) return false;
    SetText(detail, cap, L"Đã gửi StopAutoPath");
    return true;
}


bool ClickNpc(int npcID, wchar_t* detail, std::size_t cap) {
    if (npcID <= 0) { SetText(detail, cap, L"NPC ID không hợp lệ"); return false; }
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    const MethodInfo* click = ExactMethod(c.gameApi, "ClickNPC", 1, true);
    if (!click) { SetText(detail, cap, L"Không resolve đúng static LuaSystemAPI_Game.ClickNPC(1 arg)"); return false; }
    std::int32_t id = npcID;
    void* args[] = { &id };
    if (!InvokeVoid(click, nullptr, args, detail, cap)) return false;
    SetText(detail, cap, L"Đã gửi ClickNPC id="); AppendInt(detail, cap, npcID);
    return true;
}

bool MatchBytes(const std::uint8_t* address, const std::uint8_t* expected, std::size_t count) {
    if (!address || !expected) return false;
    for (std::size_t i = 0; i < count; ++i) if (address[i] != expected[i]) return false;
    return true;
}

bool ValidateFrozenBuild(bool requireExecutor, wchar_t* detail, std::size_t cap) {
    if (!g_api.module) { SetText(detail, cap, L"GameAssembly chưa resolve"); return false; }
    const auto* base = reinterpret_cast<const std::uint8_t*>(g_api.module);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) { SetText(detail, cap, L"DOS header client không hợp lệ"); return false; }
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->FileHeader.TimeDateStamp != 0x6A410C14u ||
        nt->OptionalHeader.SizeOfImage != 0x03DCB000u) {
        SetText(detail, cap, L"Client khác frozen build 0x6A410C14/0x03DCB000; chặn scanner/action RVA");
        return false;
    }
    constexpr std::uintptr_t ObjectManagerGetInstanceRva = 0x655010u;
    constexpr std::uintptr_t GRoleGetHPRva = 0x69B0B0u;
    constexpr std::uintptr_t GRoleGetMaxHPRva = 0x69C780u;
    constexpr std::uintptr_t GRoleGetNameRva = 0x69D080u;
    const std::uint8_t managerSig[] = {0x48,0x83,0xEC,0x28,0x80,0x3D,0xF7,0x2D,0x17,0x03,0x00,0x75};
    const std::uint8_t hpSig[] = {0x40,0x57,0x48,0x83,0xEC,0x20,0x80,0x3D,0x3F,0xD1,0x12,0x03};
    const std::uint8_t maxHpSig[] = {0x40,0x57,0x48,0x83,0xEC,0x20,0x80,0x3D,0x71,0xBA,0x12,0x03};
    const std::uint8_t nameSig[] = {0x40,0x57,0x48,0x83,0xEC,0x20,0x80,0x3D,0x38,0xB1,0x12,0x03};
    if (!MatchBytes(base + ObjectManagerGetInstanceRva, managerSig, sizeof(managerSig)) ||
        !MatchBytes(base + GRoleGetHPRva, hpSig, sizeof(hpSig)) ||
        !MatchBytes(base + GRoleGetMaxHPRva, maxHpSig, sizeof(maxHpSig)) ||
        !MatchBytes(base + GRoleGetNameRva, nameSig, sizeof(nameSig))) {
        SetText(detail, cap, L"Chữ ký ObjectManager/GRole không khớp; scanner bị chặn");
        return false;
    }
    if (requireExecutor) {
        constexpr std::uintptr_t GetExecutorRva = 0x523CE0u;
        constexpr std::uintptr_t ExecuteUiObjectRva = 0x521B20u;
        const std::uint8_t getSig[] = {0x48,0x83,0xEC,0x28,0x80,0x3D,0x7C,0x38,0x2A,0x03,0x00,0x75};
        const std::uint8_t execSig[] = {0x40,0x53,0x55,0x56,0x57,0x48,0x83,0xEC,0x38,0x80,0x3D,0x4D};
        if (!MatchBytes(base + GetExecutorRva, getSig, sizeof(getSig)) ||
            !MatchBytes(base + ExecuteUiObjectRva, execSig, sizeof(execSig))) {
            SetText(detail, cap, L"Chữ ký MonoBehaviourExecutor không khớp; AutoFight action bị chặn");
            return false;
        }
    }
    return true;
}

bool ReadableRange(const void* address, std::size_t bytes) {
    if (!address || bytes == 0) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(address, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return false;
    if ((mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) != 0) return false;
    const auto begin = reinterpret_cast<std::uintptr_t>(address);
    const auto regionEnd = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    return begin <= regionEnd && bytes <= regionEnd - begin;
}

template <typename T>
bool SafeLoad(const void* address, T& out) {
    if (!ReadableRange(address, sizeof(T))) return false;
    std::memcpy(&out, address, sizeof(T));
    return true;
}

bool ScanNearbyMonsters(Response& response, wchar_t* detail, std::size_t cap) {
    Classes classes{};
    if (!LoadDungeonExports(false, detail, cap) || !ResolveClasses(classes, detail, cap) ||
        !SafeForAction(classes, detail, cap)) return false;
    if (!ValidateFrozenBuild(false, detail, cap)) return false;

    constexpr std::uintptr_t ObjectManagerGetInstanceRva = 0x655010u;
    constexpr std::uintptr_t GRoleGetHPRva = 0x69B0B0u;
    constexpr std::uintptr_t GRoleGetMaxHPRva = 0x69C780u;
    constexpr std::uintptr_t GRoleGetNameRva = 0x69D080u;
    constexpr std::uintptr_t ObjectManagerSprites = 0x20u;
    constexpr std::uintptr_t DictionaryEntries = 0x18u;
    constexpr std::uintptr_t DictionaryCount = 0x20u;
    constexpr std::uintptr_t ArrayData = 0x20u;
    constexpr std::uintptr_t EntrySize = 0x18u;
    constexpr std::uintptr_t EntryKey = 0x08u;
    constexpr std::uintptr_t EntryValue = 0x10u;
    constexpr std::uintptr_t GSpriteRoleID = 0x30u;

    using GetManagerFn = Il2CppObject* (__fastcall*)(const MethodInfo*);
    using IntGetterFn = std::int32_t (__fastcall*)(Il2CppObject*, const MethodInfo*);
    using NameGetterFn = Il2CppString* (__fastcall*)(Il2CppObject*, const MethodInfo*);
    auto* base = reinterpret_cast<std::uint8_t*>(g_api.module);
    auto getManager = reinterpret_cast<GetManagerFn>(base + ObjectManagerGetInstanceRva);
    auto getHP = reinterpret_cast<IntGetterFn>(base + GRoleGetHPRva);
    auto getMaxHP = reinterpret_cast<IntGetterFn>(base + GRoleGetMaxHPRva);
    auto getName = reinterpret_cast<NameGetterFn>(base + GRoleGetNameRva);

    Il2CppObject* manager = getManager(nullptr);
    void* dictionary = nullptr;
    void* entries = nullptr;
    std::int32_t count = 0;
    if (!manager ||
        !SafeLoad(reinterpret_cast<std::uint8_t*>(manager) + ObjectManagerSprites, dictionary) || !dictionary ||
        !SafeLoad(reinterpret_cast<std::uint8_t*>(dictionary) + DictionaryEntries, entries) || !entries ||
        !SafeLoad(reinterpret_cast<std::uint8_t*>(dictionary) + DictionaryCount, count) ||
        count < 0 || count > 4096) {
        SetText(detail, cap, L"ObjectManager sprite dictionary chưa sẵn sàng/hợp lệ");
        return false;
    }

    response.monsterCount = 0;
    response.scannedEntries = 0;
    response.excludedPlayerRoles = 0;
    response.excludedOtherSprites = 0;
    response.monsterHpReadFailures = 0;
    response.monsterTruncated = 0;
    wchar_t firstAcceptedClass[40]{};
    wchar_t firstRejectedClass[40]{};
    for (std::int32_t i = 0; i < count; ++i) {
        const auto* entry = reinterpret_cast<std::uint8_t*>(entries) + ArrayData +
                            static_cast<std::uintptr_t>(i) * EntrySize;
        std::int32_t keyRoleID = 0;
        std::int32_t objectRoleID = 0;
        Il2CppObject* sprite = nullptr;
        ++response.scannedEntries;
        if (!SafeLoad(entry + EntryKey, keyRoleID) || keyRoleID <= 0 ||
            !SafeLoad(entry + EntryValue, sprite) || !sprite ||
            !SafeLoad(reinterpret_cast<std::uint8_t*>(sprite) + GSpriteRoleID, objectRoleID) ||
            objectRoleID != keyRoleID) continue;

        Il2CppClass* actual = nullptr;
        if (!SafeLoad(sprite, actual) || !actual || !ReadableRange(actual, sizeof(void*))) {
            ++response.excludedOtherSprites;
            continue;
        }
        const char* className = g_api.class_get_name(actual);
        if (!className) {
            ++response.excludedOtherSprites;
            continue;
        }

        // This is the hard boundary between the AutoBuff player scanner and the
        // dungeon monster scanner. GMonster subclasses are accepted; GRole-only
        // players and every other sprite family are excluded before HP/name reads.
        const bool provenMonster = ClassChainHasExactName(actual, "GMonster");
        if (!provenMonster) {
            if (ClassChainHasExactName(actual, "GRole")) ++response.excludedPlayerRoles;
            else ++response.excludedOtherSprites;
            if (!firstRejectedClass[0]) CopyAscii(className, firstRejectedClass, _countof(firstRejectedClass));
            continue;
        }
        if (!firstAcceptedClass[0]) CopyAscii(className, firstAcceptedClass, _countof(firstAcceptedClass));
        const bool provenGRoleBase = ClassChainHasExactName(actual, "GRole");

        std::int32_t hp = -1;
        std::int32_t maxHP = -1;
        Il2CppString* managedName = nullptr;
        MonsterHpSource hpSource = MonsterHpSource::None;
        const bool semanticVitals =
            TryScalarGetter(actual, sprite, hp, "get_HP", "get_Hp", "get_CurrentHP") &&
            TryScalarGetter(actual, sprite, maxHP, "get_MaxHP", "get_MaxHp", "get_HPMax");
        if (semanticVitals) hpSource = MonsterHpSource::SemanticGetter;
#if defined(_MSC_VER)
        if (!semanticVitals && provenGRoleBase) {
            __try {
                // GMonster is proven to be in the GRole class chain on this frozen
                // client before these donor getters are allowed to run.
                hp = getHP(sprite, nullptr);
                maxHP = getMaxHP(sprite, nullptr);
                hpSource = MonsterHpSource::GuardedGRoleSubclassRva;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                hp = -1;
                maxHP = -1;
                hpSource = MonsterHpSource::None;
            }
        }
        if (provenGRoleBase) {
            __try {
                managedName = getName(sprite, nullptr);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                managedName = nullptr;
            }
        }
#else
        if (!semanticVitals && provenGRoleBase) {
            hp = getHP(sprite, nullptr);
            maxHP = getMaxHP(sprite, nullptr);
            hpSource = MonsterHpSource::GuardedGRoleSubclassRva;
        }
        if (provenGRoleBase) managedName = getName(sprite, nullptr);
#endif
        const bool liveVitalsValid = hpSource != MonsterHpSource::None &&
                                     maxHP > 0 && hp >= 0 && hp <= maxHP;
        if (!liveVitalsValid) {
            ++response.monsterHpReadFailures;
        }
        if (response.monsterCount >= kMaxMonsterRecords) {
            response.monsterTruncated = 1;
            continue;
        }

        MonsterRecord& record = response.monsters[response.monsterCount++];
        record = {};
        record.roleID = keyRoleID;
        record.hp = hp;
        record.maxHP = maxHP;
        record.hpSource = static_cast<std::int32_t>(hpSource);
        record.validMask |= MonsterValidIdentity | MonsterValidClassProof;
        if (liveVitalsValid)
            record.validMask |= MonsterValidVitals | MonsterValidLiveVitals;
        CopyAscii(className, record.className, _countof(record.className));

        if (managedName && CopyString(managedName, record.name, _countof(record.name))) {
            record.validMask |= MonsterValidName;
        } else if (TryStringGetter(actual, sprite, record.name, _countof(record.name), "get_Name", "get_RoleName")) {
            record.validMask |= MonsterValidName;
        }

        std::int32_t value = 0;
        if (TryScalarGetter(actual, sprite, value, "get_ResID", "get_ResId", "get_TemplateID") ||
            TryScalarGetter(actual, sprite, value, "get_MonsterID", "get_MonsterResID") ||
            TryIntField(actual, sprite, value, "ResID", "resID", "m_ResID", "monsterResID") ||
            TryIntField(actual, sprite, value, "MonsterID", "monsterID", "m_MonsterID")) {
            record.resID = value;
            if (value > 0) record.validMask |= MonsterValidTemplate;
        }
        value = 0;
        if (TryScalarGetter(actual, sprite, value, "get_Type", "get_SpriteType", "get_RoleType") ||
            TryIntField(actual, sprite, value, "Type", "type", "m_Type", "ObjectType")) {
            record.type = value;
            record.validMask |= MonsterValidType;
        }
        value = liveVitalsValid && hp == 0 ? 1 : 0;
        if (TryScalarGetter(actual, sprite, value, "get_IsDeath", "get_IsDead")) {
            record.dead = value ? 1 : 0;
            record.validMask |= MonsterValidDeath;
        } else if (liveVitalsValid) {
            record.dead = hp == 0 ? 1 : 0;
            record.validMask |= MonsterValidDeath;
        }
        std::int32_t x = 0, y = 0;
        const bool gotX = TryScalarGetter(actual, sprite, x, "get_PosX", "get_X") ||
                          TryIntField(actual, sprite, x, "PosX", "posX", "m_PosX");
        const bool gotY = TryScalarGetter(actual, sprite, y, "get_PosY", "get_Y") ||
                          TryIntField(actual, sprite, y, "PosY", "posY", "m_PosY");
        if (gotX && gotY) {
            record.x = x; record.y = y; record.validMask |= MonsterValidPosition;
        }
    }

    SetText(detail, cap, L"SCAN STRICT GMonster=");
    AppendInt(detail, cap, static_cast<int>(response.monsterCount));
    Append(detail, cap, L" / entries="); AppendInt(detail, cap, static_cast<int>(response.scannedEntries));
    Append(detail, cap, L" / loại GRole="); AppendInt(detail, cap, static_cast<int>(response.excludedPlayerRoles));
    Append(detail, cap, L" / loại khác="); AppendInt(detail, cap, static_cast<int>(response.excludedOtherSprites));
    Append(detail, cap, L" / HP fail="); AppendInt(detail, cap, static_cast<int>(response.monsterHpReadFailures));
    if (firstAcceptedClass[0]) { Append(detail, cap, L" / firstMonster="); Append(detail, cap, firstAcceptedClass); }
    if (firstRejectedClass[0]) { Append(detail, cap, L" / firstRejected="); Append(detail, cap, firstRejectedClass); }
    if (response.monsterTruncated) Append(detail, cap, L" • TRUNCATED 96");
    return true;
}

bool ProveUnityMainThread(wchar_t* detail, std::size_t cap) {
    const Il2CppImage* core = g_api.get_corlib ? g_api.get_corlib() : nullptr;
    if (!core) { SetText(detail, cap, L"Không lấy được corlib để proof main thread"); return false; }
    Il2CppClass* sync = g_api.class_from_name(core, "System.Threading", "SynchronizationContext");
    Il2CppClass* threadClass = g_api.class_from_name(core, "System.Threading", "Thread");
    if (!sync || !threadClass) { SetText(detail, cap, L"Thiếu SynchronizationContext/Thread"); return false; }
    const MethodInfo* getContext = ExactMethod(sync, "get_Current", 0, true);
    const MethodInfo* getThread = ExactMethod(threadClass, "get_CurrentThread", 0, true);
    Il2CppObject* context = nullptr;
    Il2CppObject* thread = nullptr;
    if (!getContext || !getThread ||
        !InvokeObject(getContext, nullptr, context, detail, cap) || !context ||
        !InvokeObject(getThread, nullptr, thread, detail, cap) || !thread) return false;
    Il2CppClass* contextClass = g_api.object_get_class(context);
    const char* contextName = contextClass ? g_api.class_get_name(contextClass) : nullptr;
    if (!Eq(contextName, "UnitySynchronizationContext")) {
        SetText(detail, cap, L"Callback không nằm trong UnitySynchronizationContext"); return false;
    }
    std::int32_t mainId = 0, currentId = 0;
    if (!ScalarGetter(contextClass, "get_MainThreadId", context, mainId, detail, cap) ||
        !ScalarGetter(threadClass, "get_ManagedThreadId", thread, currentId, detail, cap) ||
        mainId != currentId) {
        SetText(detail, cap, L"Managed thread không phải Unity main thread"); return false;
    }
    return true;
}

struct RootHandle {
    std::uint32_t value = 0;
    ~RootHandle() { if (value && g_api.gchandle_free) g_api.gchandle_free(value); }
    bool Hold(Il2CppObject* object) {
        if (!object) return false;
        value = g_api.gchandle_new(object, false);
        return value != 0;
    }
};

bool FindLuaUi(Il2CppClass* guiApi, Il2CppObject*& ui, wchar_t* detail, std::size_t cap) {
    ui = nullptr;
    const char* oneString[] = {"System.String"};
    const MethodInfo* find = FindMethodExact(guiApi, "FindUI", oneString, 1);
    if (!find) find = FindMethodExact(guiApi, "MainFindUI", oneString, 1);
    if (!find || !StaticMethod(find)) {
        SetText(detail, cap, L"Không resolve FindUI/MainFindUI(String)"); return false;
    }
    Il2CppString* name = g_api.string_new("AutoFight_Main");
    RootHandle nameRoot;
    if (!name || !nameRoot.Hold(reinterpret_cast<Il2CppObject*>(name))) {
        SetText(detail, cap, L"Không tạo/root được tên AutoFight_Main"); return false;
    }
    void* args[] = {&name};
    void* exception = nullptr;
    ui = g_api.runtime_invoke(find, nullptr, args, &exception);
    if (exception || !ui) { SetText(detail, cap, L"Không tìm thấy AutoFight_Main"); return false; }
    return true;
}

bool SetAutoFight(bool enabled, wchar_t* detail, std::size_t cap) {
    Classes classes{};
    if (!LoadDungeonExports(true, detail, cap) || !ResolveClasses(classes, detail, cap) || !classes.guiApi ||
        !SafeForAction(classes, detail, cap) || !ProveUnityMainThread(detail, cap) ||
        !ValidateFrozenBuild(true, detail, cap)) return false;

    Il2CppObject* autoFightMain = nullptr;
    if (!FindLuaUi(classes.guiApi, autoFightMain, detail, cap)) return false;
    const Il2CppImage* core = g_api.get_corlib();
    Il2CppClass* objectClass = core ? g_api.class_from_name(core, "System", "Object") : nullptr;
    Il2CppClass* int32Class = core ? g_api.class_from_name(core, "System", "Int32") : nullptr;
    if (!objectClass || !int32Class) { SetText(detail, cap, L"Thiếu System.Object/Int32"); return false; }

    std::int32_t mode = enabled ? 1 : 0; // C_AutoModel.Train / None
    Il2CppObject* boxedMode = g_api.value_box(int32Class, &mode);
    Il2CppArray* args = g_api.array_new(objectClass, 1);
    Il2CppString* functionName = g_api.string_new("StartAutoFight");
    RootHandle modeRoot, argsRoot, functionRoot, uiRoot;
    if (!boxedMode || !args || !functionName ||
        !modeRoot.Hold(boxedMode) || !argsRoot.Hold(reinterpret_cast<Il2CppObject*>(args)) ||
        !functionRoot.Hold(reinterpret_cast<Il2CppObject*>(functionName)) || !uiRoot.Hold(autoFightMain)) {
        SetText(detail, cap, L"Không tạo/root được tham số AutoFight"); return false;
    }
    constexpr std::uintptr_t ArrayDataOffset = 0x20u;
    auto** first = reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uint8_t*>(args) + ArrayDataOffset);
    *first = boxedMode;
    MemoryBarrier();

    constexpr std::uintptr_t GetExecutorRva = 0x523CE0u;
    constexpr std::uintptr_t ExecuteUiObjectRva = 0x521B20u;
    using GetExecutorFn = Il2CppObject* (__fastcall*)(const MethodInfo*);
    using ExecuteUiObjectFn = Il2CppObject* (__fastcall*)(Il2CppObject*, Il2CppObject*, Il2CppString*, Il2CppArray*, const MethodInfo*);
    auto* base = reinterpret_cast<std::uint8_t*>(g_api.module);
    auto getExecutor = reinterpret_cast<GetExecutorFn>(base + GetExecutorRva);
    auto executeUiObject = reinterpret_cast<ExecuteUiObjectFn>(base + ExecuteUiObjectRva);
    Il2CppObject* executor = getExecutor(nullptr);
    if (!executor) { SetText(detail, cap, L"MonoBehaviourExecutor.Instance null"); return false; }
    (void)executeUiObject(executor, autoFightMain, functionName, args, nullptr);
    SetText(detail, cap, enabled
        ? L"Đã gửi AutoFight_Main.StartAutoFight(Train=1); controller phải verify AutoFight=ON"
        : L"Đã gửi AutoFight_Main.StartAutoFight(None=0); controller phải verify AutoFight=OFF");
    return true;
}

bool EnsureShared() {
    if (g_shared) return true;
    wchar_t name[96]{}; MappingName(GetCurrentProcessId(), name, _countof(name));
    g_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!g_mapping) return false;
    g_shared = reinterpret_cast<SharedBlock*>(MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    if (!g_shared || g_shared->magic != kMagic || g_shared->protocolVersion != kProtocolVersion ||
        g_shared->targetPid != GetCurrentProcessId()) {
        if (g_shared) UnmapViewOfFile(g_shared);
        if (g_mapping) CloseHandle(g_mapping);
        g_shared = nullptr; g_mapping = nullptr; return false;
    }
    InterlockedExchange(&g_shared->bridgeLoaded, 1);
    return true;
}

void ProcessRequest() {
    if (!EnsureShared()) return;
    const LONG seq = g_shared->requestSeq;
    if (seq <= 0 || seq == g_shared->completedSeq) return;
    if (InterlockedCompareExchange(&g_shared->bridgeBusy, 1, 0) != 0) return;

    Response r{};
    wchar_t detail[512]{};
    bool ok = false;
    const DWORD callbackThreadId = GetCurrentThreadId();
    if (callbackThreadId != g_shared->targetWindowThreadId) {
        SetText(detail, _countof(detail), L"Sai callback thread; action bị chặn");
    } else {
        const Command cmd = static_cast<Command>(g_shared->request.command);
        switch (cmd) {
            case Command::ReadState:
                ok = ReadState(r.snapshot, detail, _countof(detail)); break;
            case Command::ToggleRide:
                ok = ToggleRide(g_shared->request.arg0 != 0, detail, _countof(detail)); break;
            case Command::StartPath:
                ok = StartPath(g_shared->request.arg0, g_shared->request.arg1, g_shared->request.arg2,
                               detail, _countof(detail)); break;
            case Command::StopPath:
                ok = StopPath(detail, _countof(detail)); break;
            case Command::ClickNpc:
                ok = ClickNpc(g_shared->request.arg0, detail, _countof(detail)); break;
            case Command::ScanNearbyMonsters:
                ok = ScanNearbyMonsters(r, detail, _countof(detail)); break;
            case Command::SetAutoFight:
                ok = SetAutoFight(g_shared->request.arg0 != 0, detail, _countof(detail)); break;
            default:
                SetText(detail, _countof(detail), L"Command không hợp lệ"); break;
        }
    }
    r.ok = ok ? 1 : 0;
    SetText(r.detail, _countof(r.detail), detail);
    g_shared->response = r;
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
        g_shared = nullptr; g_mapping = nullptr;
    }
    return TRUE;
}
