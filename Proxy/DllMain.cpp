#include <Windows.h>
#include <cstdio>

// Global handle to the real dwmapi.dll
static HMODULE g_OriginalDwmapi = nullptr;

// Load the real dwmapi.dll from System32
static HMODULE LoadOriginalDwmapi()
{
    char systemPath[MAX_PATH];
    GetSystemDirectoryA(systemPath, MAX_PATH);

    char dwmapiPath[MAX_PATH];
    wsprintfA(dwmapiPath, "%s\\dwmapi.dll", systemPath);

#ifdef _DEBUG
    printf("[Proxy] Loading real dwmapi.dll from System32...\n");
    printf("[Proxy] Path: %s\n", dwmapiPath);
#endif

    HMODULE hDll = LoadLibraryA(dwmapiPath);

#ifdef _DEBUG
    if (hDll)
    {
        printf("[Proxy] Successfully loaded real dwmapi.dll\n");
    }
    else
    {
        printf("[Proxy] ERROR: Failed to load real dwmapi.dll (Error: %lu)\n", GetLastError());
    }
#endif

    return hDll;
}

// Load Broadsword.dll (the actual framework)
static bool LoadBroadswordFramework()
{
#ifdef _DEBUG
    printf("[Proxy] Loading Broadsword.dll...\n");
#endif

    HMODULE hBroadsword = LoadLibraryA("Broadsword.dll");

    if (hBroadsword)
    {
#ifdef _DEBUG
        printf("[Proxy] Successfully loaded Broadsword.dll\n");
#endif
        return true;
    }

#ifdef _DEBUG
    printf("[Proxy] ERROR: Failed to load Broadsword.dll (Error: %lu)\n", GetLastError());
    printf("[Proxy] Make sure Broadsword.dll is in the same folder as the game executable\n");
#endif

    return false;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
#ifdef _DEBUG
        // Allocate console for debug output
        AllocConsole();
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        SetConsoleTitleA("Broadsword Proxy - Debug Console");

        printf("Broadsword Proxy\n\n");
#endif

        // Load the real dwmapi.dll from System32
        g_OriginalDwmapi = LoadOriginalDwmapi();

        if (!g_OriginalDwmapi)
        {
#ifdef _DEBUG
            printf("[Proxy] CRITICAL ERROR: Failed to load original dwmapi.dll\n");
#endif
            return FALSE;
        }

        // Load the Broadsword framework
        bool frameworkLoaded = LoadBroadswordFramework();

#ifdef _DEBUG
        if (frameworkLoaded)
        {
            printf("[Proxy] Broadsword.dll loaded successfully\n");
        }
        else
        {
            printf("[Proxy] Failed to load Broadsword.dll\n");
        }
#endif
        break;
    }

    case DLL_PROCESS_DETACH:
        if (g_OriginalDwmapi)
        {
            FreeLibrary(g_OriginalDwmapi);
            g_OriginalDwmapi = nullptr;
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

// Macro to forward dwmapi exports to the real DLL
#define IMPLEMENT_FORWARDED_FUNCTION(name)      \
    extern "C" __declspec(dllexport) FARPROC name() \
    {                                               \
        return GetProcAddress(g_OriginalDwmapi, #name); \
    }

// Forward all dwmapi.dll exports
IMPLEMENT_FORWARDED_FUNCTION(DwmAttachMilContent)
IMPLEMENT_FORWARDED_FUNCTION(DwmDefWindowProc)
IMPLEMENT_FORWARDED_FUNCTION(DwmDetachMilContent)
IMPLEMENT_FORWARDED_FUNCTION(DwmEnableBlurBehindWindow)
IMPLEMENT_FORWARDED_FUNCTION(DwmEnableComposition)
IMPLEMENT_FORWARDED_FUNCTION(DwmEnableMMCSS)
IMPLEMENT_FORWARDED_FUNCTION(DwmExtendFrameIntoClientArea)
IMPLEMENT_FORWARDED_FUNCTION(DwmFlush)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetColorizationColor)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetCompositionTimingInfo)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetGraphicsStreamClient)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetGraphicsStreamTransformHint)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetTransportAttributes)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetUnmetTabRequirements)
IMPLEMENT_FORWARDED_FUNCTION(DwmGetWindowAttribute)
IMPLEMENT_FORWARDED_FUNCTION(DwmInvalidateIconicBitmaps)
IMPLEMENT_FORWARDED_FUNCTION(DwmIsCompositionEnabled)
IMPLEMENT_FORWARDED_FUNCTION(DwmModifyPreviousDxFrameDuration)
IMPLEMENT_FORWARDED_FUNCTION(DwmQueryThumbnailSourceSize)
IMPLEMENT_FORWARDED_FUNCTION(DwmRegisterThumbnail)
IMPLEMENT_FORWARDED_FUNCTION(DwmRenderGesture)
IMPLEMENT_FORWARDED_FUNCTION(DwmSetDxFrameDuration)
IMPLEMENT_FORWARDED_FUNCTION(DwmSetIconicLivePreviewBitmap)
IMPLEMENT_FORWARDED_FUNCTION(DwmSetIconicThumbnail)
IMPLEMENT_FORWARDED_FUNCTION(DwmSetPresentParameters)
IMPLEMENT_FORWARDED_FUNCTION(DwmSetWindowAttribute)
IMPLEMENT_FORWARDED_FUNCTION(DwmShowContact)
IMPLEMENT_FORWARDED_FUNCTION(DwmTetherContact)
IMPLEMENT_FORWARDED_FUNCTION(DwmTransitionOwnedWindow)
IMPLEMENT_FORWARDED_FUNCTION(DwmUnregisterThumbnail)
IMPLEMENT_FORWARDED_FUNCTION(DwmUpdateThumbnailProperties)
