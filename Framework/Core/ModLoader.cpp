#include "ModLoader.hpp"
#include <iostream>

namespace Broadsword {

ModLoader::~ModLoader() {
    UnloadAll();
}

size_t ModLoader::DiscoverMods(const std::filesystem::path& modsDir) {
    if (!std::filesystem::exists(modsDir)) {
        std::cerr << "[ModLoader] Mods directory does not exist: " << modsDir << "\n";
        return 0;
    }

    if (!std::filesystem::is_directory(modsDir)) {
        std::cerr << "[ModLoader] Path is not a directory: " << modsDir << "\n";
        return 0;
    }

    size_t discovered = 0;

    for (const auto& entry : std::filesystem::directory_iterator(modsDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto& path = entry.path();
        if (path.extension() != ".dll") {
            continue;
        }

        std::cout << "[ModLoader] Discovered mod: " << path.filename() << "\n";

        if (LoadMod(path)) {
            discovered++;
        }
    }

    std::cout << "[ModLoader] Loaded " << discovered << " mods\n";
    return discovered;
}

bool ModLoader::LoadMod(const std::filesystem::path& dllPath) {
    // Load DLL
    HMODULE hModule = LoadLibraryW(dllPath.c_str());
    if (!hModule) {
        std::cerr << "[ModLoader] Failed to load DLL: " << dllPath << " (Error: " << GetLastError() << ")\n";
        return false;
    }

    // Find CreateMod export
    using CreateModFunc = Mod* (*)();
    auto CreateModPtr = reinterpret_cast<CreateModFunc>(GetProcAddress(hModule, "CreateMod"));
    if (!CreateModPtr) {
        std::cerr << "[ModLoader] DLL missing CreateMod export: " << dllPath << "\n";
        FreeLibrary(hModule);
        return false;
    }

    // Find DestroyMod export
    using DestroyModFunc = void (*)(Mod*);
    auto DestroyModPtr = reinterpret_cast<DestroyModFunc>(GetProcAddress(hModule, "DestroyMod"));
    if (!DestroyModPtr) {
        std::cerr << "[ModLoader] DLL missing DestroyMod export: " << dllPath << "\n";
        FreeLibrary(hModule);
        return false;
    }

    // Create mod instance
    Mod* modInstance = CreateModPtr();
    if (!modInstance) {
        std::cerr << "[ModLoader] CreateMod returned null: " << dllPath << "\n";
        FreeLibrary(hModule);
        return false;
    }

    // Get mod info
    ModInfo info = modInstance->GetInfo();
    std::cout << "[ModLoader] Loaded mod: " << info.Name << " v" << info.Version << " by " << info.Author << "\n";

    // Store loaded mod
    LoadedMod loadedMod;
    loadedMod.hModule = hModule;
    loadedMod.modInstance = modInstance;
    loadedMod.dllPath = dllPath;
    loadedMod.CreateMod = CreateModPtr;
    loadedMod.DestroyMod = DestroyModPtr;

    m_LoadedMods.push_back(loadedMod);

    return true;
}

void ModLoader::UnloadAll() {
    std::cout << "[ModLoader] Unloading " << m_LoadedMods.size() << " mods...\n";

    for (auto& loadedMod : m_LoadedMods) {
        if (loadedMod.modInstance) {
            // Call OnUnregister
            try {
                loadedMod.modInstance->OnUnregister();
            } catch (const std::exception& e) {
                std::cerr << "[ModLoader] Exception in OnUnregister: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[ModLoader] Unknown exception in OnUnregister\n";
            }

            // Destroy mod instance
            if (loadedMod.DestroyMod) {
                loadedMod.DestroyMod(loadedMod.modInstance);
            }
        }

        // Unload DLL
        if (loadedMod.hModule) {
            FreeLibrary(loadedMod.hModule);
        }
    }

    m_LoadedMods.clear();
    std::cout << "[ModLoader] All mods unloaded\n";
}

std::vector<Mod*> ModLoader::GetLoadedMods() const {
    std::vector<Mod*> mods;
    mods.reserve(m_LoadedMods.size());

    for (const auto& loadedMod : m_LoadedMods) {
        mods.push_back(loadedMod.modInstance);
    }

    return mods;
}

void ModLoader::RegisterAllMods(ModContext& ctx) {
    std::cout << "[ModLoader] Registering " << m_LoadedMods.size() << " mods...\n";

    for (auto& loadedMod : m_LoadedMods) {
        if (loadedMod.modInstance) {
            try {
                ModInfo info = loadedMod.modInstance->GetInfo();
                std::cout << "[ModLoader] Registering mod: " << info.Name << "\n";
                loadedMod.modInstance->OnRegister(ctx);
                std::cout << "[ModLoader] Successfully registered: " << info.Name << "\n";
            } catch (const std::exception& e) {
                std::cerr << "[ModLoader] Exception in OnRegister: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[ModLoader] Unknown exception in OnRegister\n";
            }
        }
    }

    std::cout << "[ModLoader] All mods registered\n";
}

} // namespace Broadsword
