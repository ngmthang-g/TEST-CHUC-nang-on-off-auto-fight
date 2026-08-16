#include <windows.h>
#include <cstdint>
#include <cstddef>
#include "protocol.h"

using namespace autofighttest;

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
    FARPROC raw = GetProcAddress(module, name);
    if (!raw) return false;
    static_assert(sizeof(raw) == sizeof(out), "pointer-size mismatch");
    const auto* src = reinterpret_cast<const unsigned char*>(&raw);
    auto* dst = reinterpret_cast<unsigned char*>(&out);
    for (std::size_t i = 0; i < sizeof(out); ++i) dst[i] = src[i];
    return out != nullptr;
}

bool Eq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (*a++ != *b++) return false;
    }
    return *a == *b;
}

void SetText(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || cap == 0) return;
    std::size_t i = 0;
    if (text) {
        while (i + 1 < cap && text[i]) {
            out[i] = text[i];
            ++i;
        }
    }
    out[i] = 0;
}

void Append(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || !text || cap == 0) return;
    std::size_t n = 0;
    while (n + 1 < cap && out[n]) ++n;
    std::size_t i = 0;
    while (n + 1 < cap && text[i]) out[n++] = text[i++];
    out[n] = 0;
}

void AppendAscii(wchar_t* out, std::size_t cap, const char* text) {
    if (!out || !text || cap == 0) return;
    std::size_t n = 0;
    while (n + 1 < cap && out[n]) ++n;
    std::size_t i = 0;
    while (n + 1 < cap && text[i]) {
        const unsigned char ch = static_cast<unsigned char>(text[i++]);
        out[n++] = static_cast<wchar_t>(ch);
    }
    out[n] = 0;
}

struct Api {
    HMODULE module = nullptr;
    Il2CppDomain* (__cdecl* domain_get)() = nullptr;
    const Il2CppAssembly* (__cdecl* domain_assembly_open)(Il2CppDomain*, const char*) = nullptr;
    const Il2CppImage* (__cdecl* assembly_get_image)(const Il2CppAssembly*) = nullptr;
    Il2CppClass* (__cdecl* class_from_name)(const Il2CppImage*, const char*, const char*) = nullptr;
    Il2CppClass* (__cdecl* class_get_parent)(Il2CppClass*) = nullptr;
    const MethodInfo* (__cdecl* class_get_method_from_name)(Il2CppClass*, const char*, int) = nullptr;
    const MethodInfo* (__cdecl* class_get_methods)(Il2CppClass*, void**) = nullptr;
    const char* (__cdecl* method_get_name)(const MethodInfo*) = nullptr;
    FieldInfo* (__cdecl* class_get_field_from_name)(Il2CppClass*, const char*) = nullptr;
    std::uint32_t (__cdecl* field_get_flags)(FieldInfo*) = nullptr;
    void (__cdecl* field_static_get_value)(FieldInfo*, void*) = nullptr;
    std::uint32_t (__cdecl* method_get_flags)(const MethodInfo*, std::uint32_t*) = nullptr;
    std::uint32_t (__cdecl* method_get_param_count)(const MethodInfo*) = nullptr;
    const Il2CppType* (__cdecl* method_get_param)(const MethodInfo*, std::uint32_t) = nullptr;
    const Il2CppType* (__cdecl* method_get_return_type)(const MethodInfo*) = nullptr;
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, void**) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppString* (__cdecl* string_new)(const char*) = nullptr;
    Il2CppArray* (__cdecl* array_new)(Il2CppClass*, std::uintptr_t) = nullptr;

    bool Load(wchar_t* detail, std::size_t cap) {
        if (module) return true;
        module = GetModuleHandleW(L"GameAssembly.dll");
        if (!module) {
            SetText(detail, cap, L"GameAssembly.dll chưa sẵn sàng");
            return false;
        }
#define NEED(symbol) do { \
    if (!Resolve(module, "il2cpp_" #symbol, symbol)) { \
        SetText(detail, cap, L"Thiếu IL2CPP export bắt buộc"); \
        return false; \
    } \
} while (0)
        NEED(domain_get);
        NEED(domain_assembly_open);
        NEED(assembly_get_image);
        NEED(class_from_name);
        NEED(class_get_parent);
        NEED(class_get_method_from_name);
        NEED(class_get_methods);
        NEED(method_get_name);
        NEED(class_get_field_from_name);
        NEED(field_static_get_value);
        NEED(method_get_flags);
        NEED(method_get_param_count);
        NEED(method_get_param);
        NEED(method_get_return_type);
        NEED(type_get_name);
        NEED(runtime_invoke);
        NEED(object_unbox);
        NEED(string_new);
        NEED(array_new);
#undef NEED
        (void)Resolve(module, "il2cpp_field_get_flags", field_get_flags);
        if (!Resolve(module, "il2cpp_free", free_fn)) {
            SetText(detail, cap, L"Thiếu il2cpp_free");
            return false;
        }
        return true;
    }
};

Api g_api;

const Il2CppImage* AssemblyImage(const char* name, const char* fallback = nullptr) {
    Il2CppDomain* domain = g_api.domain_get ? g_api.domain_get() : nullptr;
    if (!domain || !name) return nullptr;
    const Il2CppAssembly* assembly = g_api.domain_assembly_open(domain, name);
    if (!assembly && fallback) assembly = g_api.domain_assembly_open(domain, fallback);
    return assembly ? g_api.assembly_get_image(assembly) : nullptr;
}

const Il2CppImage* GameImage() {
    return AssemblyImage("Assembly-CSharp", "Assembly-CSharp.dll");
}

bool StaticMethod(const MethodInfo* method) {
    if (!method) return false;
    constexpr std::uint32_t StaticFlag = 0x0010;
    std::uint32_t iflags = 0;
    return (g_api.method_get_flags(method, &iflags) & StaticFlag) != 0;
}

const MethodInfo* FindMethod(Il2CppClass* klass, const char* name, int argc) {
    for (Il2CppClass* c = klass; c; c = g_api.class_get_parent(c)) {
        if (const MethodInfo* method = g_api.class_get_method_from_name(c, name, argc)) return method;
    }
    return nullptr;
}

bool ParamType(const MethodInfo* method, std::uint32_t index, const char* expected) {
    if (!method || index >= g_api.method_get_param_count(method)) return false;
    const Il2CppType* type = g_api.method_get_param(method, index);
    char* name = type ? g_api.type_get_name(type) : nullptr;
    if (!name) return false;
    const bool ok = Eq(name, expected);
    g_api.free_fn(name);
    return ok;
}

const MethodInfo* FindMethodExact(Il2CppClass* klass, const char* name,
                                  const char* const* paramTypes, std::uint32_t count) {
    if (!klass || !name || !g_api.class_get_methods || !g_api.method_get_name) return nullptr;
    for (Il2CppClass* c = klass; c; c = g_api.class_get_parent(c)) {
        void* iter = nullptr;
        while (const MethodInfo* method = g_api.class_get_methods(c, &iter)) {
            const char* methodName = g_api.method_get_name(method);
            if (!Eq(methodName, name) || g_api.method_get_param_count(method) != count) continue;
            bool match = true;
            for (std::uint32_t i = 0; i < count; ++i) {
                if (!ParamType(method, i, paramTypes[i])) {
                    match = false;
                    break;
                }
            }
            if (match) return method;
        }
    }
    return nullptr;
}

void AppendExecuteFunctionSignatures(Il2CppClass* klass, wchar_t* detail, std::size_t cap) {
    SetText(detail, cap, L"ExecuteFunction overloads: ");
    std::uint32_t shown = 0;
    for (Il2CppClass* c = klass; c && shown < 8; c = g_api.class_get_parent(c)) {
        void* iter = nullptr;
        while (shown < 8) {
            const MethodInfo* method = g_api.class_get_methods(c, &iter);
            if (!method) break;
            const char* methodName = g_api.method_get_name(method);
            if (!Eq(methodName, "ExecuteFunction")) continue;
            if (shown > 0) Append(detail, cap, L" | ");
            Append(detail, cap, StaticMethod(method) ? L"static(" : L"inst(");
            const std::uint32_t count = g_api.method_get_param_count(method);
            for (std::uint32_t i = 0; i < count; ++i) {
                if (i > 0) Append(detail, cap, L",");
                const Il2CppType* type = g_api.method_get_param(method, i);
                char* typeName = type ? g_api.type_get_name(type) : nullptr;
                if (typeName) {
                    AppendAscii(detail, cap, typeName);
                    g_api.free_fn(typeName);
                } else {
                    Append(detail, cap, L"?");
                }
            }
            Append(detail, cap, L")");
            const Il2CppType* returnType = g_api.method_get_return_type(method);
            char* returnName = returnType ? g_api.type_get_name(returnType) : nullptr;
            if (returnName) {
                Append(detail, cap, L"->");
                AppendAscii(detail, cap, returnName);
                g_api.free_fn(returnName);
            }
            ++shown;
        }
    }
    if (shown == 0) Append(detail, cap, L"NONE");
}

bool InvokeBool(const MethodInfo* method, void* instance, bool& out, wchar_t* detail, std::size_t cap) {
    out = false;
    if (!method) {
        SetText(detail, cap, L"Bool getter chưa resolve");
        return false;
    }
    const Il2CppType* returnType = g_api.method_get_return_type(method);
    char* typeName = returnType ? g_api.type_get_name(returnType) : nullptr;
    if (!typeName || !Eq(typeName, "System.Boolean")) {
        if (typeName) g_api.free_fn(typeName);
        SetText(detail, cap, L"Bool getter có return type không đúng");
        return false;
    }
    g_api.free_fn(typeName);
    void* exception = nullptr;
    Il2CppObject* boxed = g_api.runtime_invoke(method, instance, nullptr, &exception);
    if (exception || !boxed) {
        SetText(detail, cap, L"Bool getter ném exception/null");
        return false;
    }
    void* raw = g_api.object_unbox(boxed);
    if (!raw) {
        SetText(detail, cap, L"Không unbox được Boolean");
        return false;
    }
    out = *reinterpret_cast<const std::uint8_t*>(raw) != 0;
    return true;
}
