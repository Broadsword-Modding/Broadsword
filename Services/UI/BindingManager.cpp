#include "BindingManager.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <Windows.h>
#include <iostream>

namespace Broadsword {

// ========================================
// BindingID Implementation
// ========================================

size_t BindingID::Hash() const {
    // Combine hashes of all fields
    size_t h1 = std::hash<std::string>{}(modName);
    size_t h2 = std::hash<std::string>{}(fileName);
    size_t h3 = std::hash<uint32_t>{}(line);
    size_t h4 = std::hash<uint32_t>{}(column);
    size_t h5 = std::hash<std::string>{}(label);

    // Simple hash combine
    size_t result = h1;
    result ^= h2 + 0x9e3779b9 + (result << 6) + (result >> 2);
    result ^= h3 + 0x9e3779b9 + (result << 6) + (result >> 2);
    result ^= h4 + 0x9e3779b9 + (result << 6) + (result >> 2);
    result ^= h5 + 0x9e3779b9 + (result << 6) + (result >> 2);

    return result;
}

std::string BindingID::ToString() const {
    return modName + "::" + label + " (" + fileName + ":" + std::to_string(line) + ")";
}

BindingID BindingID::FromLocation(
    std::string_view modName,
    std::string_view label,
    std::source_location loc
) {
    BindingID id;
    id.modName = modName;
    id.fileName = std::filesystem::path(loc.file_name()).filename().string();
    id.line = loc.line();
    id.column = loc.column();
    id.label = label;
    return id;
}

bool BindingID::operator==(const BindingID& other) const {
    return modName == other.modName &&
           fileName == other.fileName &&
           line == other.line &&
           column == other.column &&
           label == other.label;
}

bool BindingID::operator!=(const BindingID& other) const {
    return !(*this == other);
}

// ========================================
// BindingManager Implementation
// ========================================

void BindingManager::Register(const BindingID& id, std::string_view displayLabel) {
    size_t hash = id.Hash();

    // Check if already registered
    if (m_Bindings.find(hash) != m_Bindings.end()) {
        return; // Already registered
    }

    // Create new binding
    Binding binding;
    binding.id = id;
    binding.displayLabel = displayLabel;
    binding.vkey = std::nullopt;

    m_Bindings[hash] = binding;

    std::cout << "[BindingManager] Registered: " << id.ToString() << "\n";
}

void BindingManager::Bind(const BindingID& id, int vkey) {
    size_t hash = id.Hash();

    auto it = m_Bindings.find(hash);
    if (it == m_Bindings.end()) {
        std::cerr << "[BindingManager] Cannot bind unregistered action: " << id.ToString() << "\n";
        return;
    }

    it->second.vkey = vkey;
    std::cout << "[BindingManager] Bound " << id.label << " to VK " << vkey << "\n";
}

void BindingManager::Unbind(const BindingID& id) {
    size_t hash = id.Hash();

    auto it = m_Bindings.find(hash);
    if (it == m_Bindings.end()) {
        return;
    }

    it->second.vkey = std::nullopt;
    std::cout << "[BindingManager] Unbound " << id.label << "\n";
}

bool BindingManager::WasKeyPressed(const BindingID& id) {
    size_t hash = id.Hash();

    auto it = m_Bindings.find(hash);
    if (it == m_Bindings.end()) {
        return false;
    }

    return it->second.wasPressed;
}

std::optional<int> BindingManager::GetKey(const BindingID& id) const {
    size_t hash = id.Hash();

    auto it = m_Bindings.find(hash);
    if (it == m_Bindings.end()) {
        return std::nullopt;
    }

    return it->second.vkey;
}

std::string BindingManager::GetDisplayLabel(const BindingID& id) const {
    size_t hash = id.Hash();

    auto it = m_Bindings.find(hash);
    if (it == m_Bindings.end()) {
        return "";
    }

    return it->second.displayLabel;
}

bool BindingManager::IsRegistered(const BindingID& id) const {
    size_t hash = id.Hash();
    return m_Bindings.find(hash) != m_Bindings.end();
}

void BindingManager::Update() {
    for (auto& [hash, binding] : m_Bindings) {
        if (!binding.vkey.has_value()) {
            binding.wasPressed = false;
            binding.isPressedNow = false;
            continue;
        }

        int vkey = binding.vkey.value();
        bool isDown = IsKeyDown(vkey);

        // Detect rising edge (key was released last frame, pressed this frame)
        binding.wasPressed = !binding.isPressedNow && isDown;
        binding.isPressedNow = isDown;
    }
}

bool BindingManager::Save(const std::filesystem::path& path) const {
    try {
        nlohmann::json root = nlohmann::json::object();

        for (const auto& [hash, binding] : m_Bindings) {
            if (!binding.vkey.has_value()) {
                continue; // Don't save unbound keys
            }

            nlohmann::json bindingJson;
            bindingJson["modName"] = binding.id.modName;
            bindingJson["fileName"] = binding.id.fileName;
            bindingJson["line"] = binding.id.line;
            bindingJson["column"] = binding.id.column;
            bindingJson["label"] = binding.id.label;
            bindingJson["vkey"] = binding.vkey.value();

            // Use hash as key for fast lookup
            root[std::to_string(hash)] = bindingJson;
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "[BindingManager] Failed to open file for writing: " << path << "\n";
            return false;
        }

        file << root.dump(2);
        file.close();

        std::cout << "[BindingManager] Saved " << m_Bindings.size() << " bindings to " << path << "\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[BindingManager] Save failed: " << e.what() << "\n";
        return false;
    }
}

bool BindingManager::Load(const std::filesystem::path& path) {
    try {
        if (!std::filesystem::exists(path)) {
            std::cout << "[BindingManager] Bindings file does not exist: " << path << "\n";
            return false;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[BindingManager] Failed to open file for reading: " << path << "\n";
            return false;
        }

        nlohmann::json root;
        file >> root;
        file.close();

        int loadedCount = 0;

        for (auto& [hashStr, bindingJson] : root.items()) {
            // Reconstruct BindingID
            BindingID id;
            id.modName = bindingJson["modName"];
            id.fileName = bindingJson["fileName"];
            id.line = bindingJson["line"];
            id.column = bindingJson["column"];
            id.label = bindingJson["label"];

            size_t hash = id.Hash();

            // Check if this binding is registered
            auto it = m_Bindings.find(hash);
            if (it == m_Bindings.end()) {
                // Binding not registered yet, skip
                continue;
            }

            // Load key binding
            int vkey = bindingJson["vkey"];
            it->second.vkey = vkey;
            loadedCount++;
        }

        std::cout << "[BindingManager] Loaded " << loadedCount << " bindings from " << path << "\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[BindingManager] Load failed: " << e.what() << "\n";
        return false;
    }
}

std::unordered_map<size_t, std::string> BindingManager::GetAllBindings() const {
    std::unordered_map<size_t, std::string> result;

    for (const auto& [hash, binding] : m_Bindings) {
        result[hash] = binding.displayLabel;
    }

    return result;
}

bool BindingManager::IsKeyDown(int vkey) const {
    // Use GetAsyncKeyState to check if key is currently down
    // High-order bit is set if key is down
    return (GetAsyncKeyState(vkey) & 0x8000) != 0;
}

} // namespace Broadsword
