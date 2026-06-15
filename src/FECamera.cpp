// FECameraCore — NFS ProStreet
// Pointer chain + read/write for the FE camera struct.
//
// Pointer: [nfs.exe + 0x00BFAD24] -> FE camera struct

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <cstddef>

// ─────────────────────────────────────────────────────────────
// Camera struct layout
// ─────────────────────────────────────────────────────────────
namespace Off {
    constexpr uintptr_t OrbitV  = 0x9C;
    constexpr uintptr_t OrbitH  = 0xA0;
    constexpr uintptr_t OrbitR  = 0xA4;
    constexpr uintptr_t Roll    = 0xA8;
    constexpr uintptr_t FOV     = 0xAC;
    constexpr uintptr_t LookX   = 0xBC;
    constexpr uintptr_t LookY   = 0xC0;
    constexpr uintptr_t LookZ   = 0xC4;
    constexpr uintptr_t Default = 0x80;   // pristine value offset
}

static constexpr uintptr_t PTR_OFFSET = 0x00BFAD24u;

static uintptr_t g_nfsBase = 0;

static bool SafeRead(const void* p, size_t n) {
    __try {
        volatile BYTE a = *static_cast<const BYTE*>(p);
        volatile BYTE b = *(static_cast<const BYTE*>(p) + n - 1);
        (void)a; (void)b;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static uintptr_t GetCamStruct() {
    if (!g_nfsBase) return 0;
    uintptr_t addr = g_nfsBase + PTR_OFFSET;
    if (!SafeRead(reinterpret_cast<const void*>(addr), sizeof(uintptr_t)))
        return 0;
    return *reinterpret_cast<uintptr_t*>(addr);
}

static float ReadF(uintptr_t off) {
    uintptr_t s = GetCamStruct();
    return s ? *reinterpret_cast<float*>(s + off) : 0.f;
}

static void WriteF(uintptr_t off, float val) {
    uintptr_t s = GetCamStruct();
    if (s) *reinterpret_cast<float*>(s + off) = val;
}

static void ResetToDefault() {
    uintptr_t s = GetCamStruct();
    if (!s) return;

    static constexpr uintptr_t kFields[] = {
        Off::OrbitV, Off::OrbitH, Off::OrbitR, Off::Roll,
        Off::FOV, Off::LookX, Off::LookY, Off::LookZ
    };

    for (size_t i = 0; i < std::size(kFields); ++i)
        WriteF(kFields[i], ReadF(kFields[i] + Off::Default));
}

// ─────────────────────────────────────────────────────────────
// DLL entry
// ─────────────────────────────────────────────────────────────
static DWORD WINAPI Init(LPVOID) {
    g_nfsBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("nfs.exe"));
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hMod);
        CloseHandle(CreateThread(nullptr, 0, Init, nullptr, 0, nullptr));
    }
    return TRUE;
}
