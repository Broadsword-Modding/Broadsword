#include <Windows.h>

// Global handle to the real dwmapi.dll
static HMODULE g_OriginalDwmapi = nullptr;

// Load the real dwmapi.dll from System32
static HMODULE LoadOriginalDwmapi()
{
    char systemPath[MAX_PATH];
    GetSystemDirectoryA(systemPath, MAX_PATH);

    char dwmapiPath[MAX_PATH];
    wsprintfA(dwmapiPath, "%s\\dwmapi.dll", systemPath);

    return LoadLibraryA(dwmapiPath);
}

// Load Broadsword.dll (the actual framework)
static bool LoadBroadswordFramework()
{
    HMODULE hBroadsword = LoadLibraryA("Broadsword.dll");

    if (hBroadsword)
    {
        return true;
    }

    MessageBoxA(nullptr, "Could not find 'Broadsword.dll'.\n\n"
                         "Please make sure the file is in the same folder as the game executable.",
                "Broadsword Framework", MB_OK | MB_ICONERROR);

    return false;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Load the real dwmapi.dll from System32
        g_OriginalDwmapi = LoadOriginalDwmapi();

        if (!g_OriginalDwmapi)
        {
            MessageBoxA(nullptr, "Failed to load original dwmapi.dll from System32.", "Broadsword Framework",
                        MB_OK | MB_ICONERROR);
            return FALSE;
        }

        // Load the Broadsword framework
        LoadBroadswordFramework();
        break;

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
