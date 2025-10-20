#pragma once

#include "../../ModAPI/Mod.hpp"
#include <filesystem>
#include <vector>
#include <memory>
#include <Windows.h>

namespace Broadsword {

/**
 * ModLoader - Discovers and manages mod DLLs
 *
 * Lifecycle:
 * 1. DiscoverMods() - Scans Mods/ folder for DLLs
 * 2. LoadMod() - Loads each DLL, calls CreateMod(), OnRegister()
 * 3. GetLoadedMods() - Returns list of active mods
 * 4. UnloadAll() - Calls OnUnregister(), DestroyMod(), FreeLibrary()
 *
 * DLL Export Requirements:
 * - extern "C" __declspec(dllexport) Broadsword::Mod* CreateMod();
 * - extern "C" __declspec(dllexport) void DestroyMod(Broadsword::Mod* mod);
 *
 * Thread Safety:
 * - All methods run on game thread
 * - No synchronization needed (single-threaded)
 */
class ModLoader {
public:
    ModLoader() = default;
    ~ModLoader();

    /**
     * Discover all mod DLLs in the specified directory
     *
     * @param modsDir Path to Mods/ folder
     * @return Number of DLLs discovered
     */
    size_t DiscoverMods(const std::filesystem::path& modsDir);

    /**
     * Load a single mod DLL
     *
     * Steps:
     * 1. LoadLibrary to load DLL
     * 2. GetProcAddress to find CreateMod export
     * 3. Call CreateMod() to instantiate mod
     * 4. Store in m_LoadedMods
     *
     * Note: OnRegister() is NOT called here - it's called separately
     * after all mods are loaded to allow proper initialization order.
     *
     * @param dllPath Path to mod DLL
     * @return true if mod loaded successfully, false otherwise
     */
    bool LoadMod(const std::filesystem::path& dllPath);

    /**
     * Unload all loaded mods
     *
     * Steps:
     * 1. Call OnUnregister() for each mod
     * 2. Call DestroyMod() for each mod
     * 3. FreeLibrary to unload DLL
     * 4. Clear m_LoadedMods
     */
    void UnloadAll();

    /**
     * Get list of loaded mods
     *
     * @return Vector of Mod pointers (non-owning)
     */
    std::vector<Mod*> GetLoadedMods() const;

    /**
     * Get number of loaded mods
     */
    size_t GetModCount() const { return m_LoadedMods.size(); }

    /**
     * Register all loaded mods
     *
     * Calls OnRegister() for each mod with ModContext.
     * Should be called after all framework services are initialized.
     *
     * @param ctx ModContext with references to services
     */
    void RegisterAllMods(ModContext& ctx);

private:
    /**
     * Loaded mod tracking
     */
    struct LoadedMod {
        HMODULE hModule;               // DLL handle from LoadLibrary
        Mod* modInstance;              // Mod instance from CreateMod()
        std::filesystem::path dllPath; // Path to DLL for logging

        using CreateModFunc = Mod* (*)();
        using DestroyModFunc = void (*)(Mod*);

        CreateModFunc CreateMod;   // Function pointer to CreateMod export
        DestroyModFunc DestroyMod; // Function pointer to DestroyMod export
    };

    std::vector<LoadedMod> m_LoadedMods;
};

} // namespace Broadsword
