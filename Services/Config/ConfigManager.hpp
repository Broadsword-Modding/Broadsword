#pragma once

#include "Reflection.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>

namespace Broadsword {

/**
 * ConfigManager - Automatic config serialization using reflection
 *
 * Serializes and deserializes config structs to/from JSON.
 * Uses FieldDescriptor registration until C++26 reflection is available.
 *
 * Features:
 * - Type-safe serialization for bool, int, float, double, string, enum
 * - Automatic JSON conversion
 * - Missing field handling with defaults
 * - Save/Load from filesystem
 *
 * Usage:
 *   struct MyConfig {
 *       [[configurable]] bool godMode = false;
 *       [[configurable]] float speed = 1.0f;
 *
 *       static auto GetConfigFields() {
 *           return std::array{
 *               FieldDescriptor{"godMode", offsetof(MyConfig, godMode), FieldType::Bool},
 *               FieldDescriptor{"speed", offsetof(MyConfig, speed), FieldType::Float},
 *           };
 *       }
 *   };
 *
 *   MyConfig config;
 *   ConfigManager::Save(config, config.GetConfigFields(), "config.json");
 *   ConfigManager::Load(config, config.GetConfigFields(), "config.json");
 */
class ConfigManager {
public:
    /**
     * Serialize config to JSON
     *
     * @param instance Pointer to config instance
     * @param fields Span of field descriptors
     * @return JSON object
     */
    template<typename T>
    static nlohmann::json Serialize(T* instance, std::span<const FieldDescriptor> fields) {
        nlohmann::json json;

        for (const auto& field : fields) {
            // Calculate field address
            uint8_t* basePtr = reinterpret_cast<uint8_t*>(instance);
            void* fieldPtr = basePtr + field.offset;

            // Serialize based on type
            switch (field.type) {
                case FieldType::Bool:
                    json[field.name] = *reinterpret_cast<bool*>(fieldPtr);
                    break;

                case FieldType::Int:
                    json[field.name] = *reinterpret_cast<int*>(fieldPtr);
                    break;

                case FieldType::Float:
                    json[field.name] = *reinterpret_cast<float*>(fieldPtr);
                    break;

                case FieldType::Double:
                    json[field.name] = *reinterpret_cast<double*>(fieldPtr);
                    break;

                case FieldType::String:
                    json[field.name] = *reinterpret_cast<std::string*>(fieldPtr);
                    break;

                case FieldType::Enum:
                    // Treat enums as int for now
                    json[field.name] = *reinterpret_cast<int*>(fieldPtr);
                    break;
            }
        }

        return json;
    }

    /**
     * Deserialize JSON to config
     *
     * @param instance Pointer to config instance
     * @param fields Span of field descriptors
     * @param json JSON object to deserialize from
     */
    template<typename T>
    static void Deserialize(T* instance, std::span<const FieldDescriptor> fields, const nlohmann::json& json) {
        for (const auto& field : fields) {
            // Check if field exists in JSON
            if (!json.contains(field.name)) {
                // Field missing, keep default value
                continue;
            }

            // Calculate field address
            uint8_t* basePtr = reinterpret_cast<uint8_t*>(instance);
            void* fieldPtr = basePtr + field.offset;

            try {
                // Deserialize based on type
                switch (field.type) {
                    case FieldType::Bool:
                        *reinterpret_cast<bool*>(fieldPtr) = json[field.name].get<bool>();
                        break;

                    case FieldType::Int:
                        *reinterpret_cast<int*>(fieldPtr) = json[field.name].get<int>();
                        break;

                    case FieldType::Float:
                        *reinterpret_cast<float*>(fieldPtr) = json[field.name].get<float>();
                        break;

                    case FieldType::Double:
                        *reinterpret_cast<double*>(fieldPtr) = json[field.name].get<double>();
                        break;

                    case FieldType::String:
                        *reinterpret_cast<std::string*>(fieldPtr) = json[field.name].get<std::string>();
                        break;

                    case FieldType::Enum:
                        // Treat enums as int for now
                        *reinterpret_cast<int*>(fieldPtr) = json[field.name].get<int>();
                        break;
                }
            } catch (const std::exception& e) {
                std::cerr << "[ConfigManager] Failed to deserialize field " << field.name << ": " << e.what() << "\n";
            }
        }
    }

    /**
     * Save config to file
     *
     * @param instance Pointer to config instance
     * @param fields Span of field descriptors
     * @param path File path
     * @return true if save succeeded
     */
    template<typename T>
    static bool Save(T* instance, std::span<const FieldDescriptor> fields, const std::filesystem::path& path) {
        try {
            nlohmann::json json = Serialize(instance, fields);

            std::ofstream file(path);
            if (!file.is_open()) {
                std::cerr << "[ConfigManager] Failed to open file for writing: " << path << "\n";
                return false;
            }

            file << json.dump(2);
            file.close();

            std::cout << "[ConfigManager] Saved config to " << path << "\n";
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Save failed: " << e.what() << "\n";
            return false;
        }
    }

    /**
     * Load config from file
     *
     * @param instance Pointer to config instance
     * @param fields Span of field descriptors
     * @param path File path
     * @return true if load succeeded
     */
    template<typename T>
    static bool Load(T* instance, std::span<const FieldDescriptor> fields, const std::filesystem::path& path) {
        try {
            if (!std::filesystem::exists(path)) {
                std::cout << "[ConfigManager] Config file does not exist: " << path << "\n";
                return false;
            }

            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "[ConfigManager] Failed to open file for reading: " << path << "\n";
                return false;
            }

            nlohmann::json json;
            file >> json;
            file.close();

            Deserialize(instance, fields, json);

            std::cout << "[ConfigManager] Loaded config from " << path << "\n";
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[ConfigManager] Load failed: " << e.what() << "\n";
            return false;
        }
    }
};

} // namespace Broadsword
