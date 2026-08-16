#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace autofighttest {

constexpr std::uint32_t kMagic = 0x54464154u; // TAFT
constexpr std::uint32_t kProtocolVersion = 0x00020100u;
constexpr UINT kWakeMessage = WM_APP + 0x531;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongAutoFightTest_";

enum class Command : std::uint32_t {
    None = 0,
    StartAutoFight = 1,
    StopAutoFight = 2,
    Probe = 3,
};

struct Request {
    std::uint32_t command = 0;
};

struct Response {
    std::int32_t ok = 0;
    std::int32_t errorCode = 0;
    std::uint32_t callbackThreadId = 0;
    wchar_t detail[512]{};
};

struct SharedBlock {
    std::uint32_t magic = kMagic;
    std::uint32_t protocolVersion = kProtocolVersion;
    std::uint32_t targetPid = 0;
    std::uint32_t targetWindowThreadId = 0;
    volatile LONG requestSeq = 0;
    volatile LONG completedSeq = 0;
    volatile LONG bridgeLoaded = 0;
    volatile LONG bridgeBusy = 0;
    Request request{};
    Response response{};
};

inline void MappingName(DWORD pid, wchar_t* output, std::size_t count) {
    if (!output || count == 0) return;
    wsprintfW(output, L"%s%lu", kMappingPrefix, static_cast<unsigned long>(pid));
}

} // namespace autofighttest
