#pragma once

#include <string>

namespace Broadsword {

/**
 * Framework error codes
 * Used with Result<T, Error> for type-safe error handling
 */
enum class Error {
    // World errors
    WorldNotLoaded,
    WorldInvalid,

    // Player errors
    PlayerNotFound,
    PlayerNotSpawned,

    // Actor errors
    ActorNotFound,
    InvalidActorClass,

    // Spawn errors
    SpawnFailed,
    InvalidSpawnLocation,

    // General errors
    InvalidOperation,
    NullPointer,
    OutOfBounds,

    // Hook errors
    HookFailed,
    HookNotFound,

    // Config errors
    ConfigLoadFailed,
    ConfigSaveFailed,
    InvalidConfigValue
};

/**
 * Convert error code to human-readable string
 */
inline const char* ToString(Error error) {
    switch (error) {
        case Error::WorldNotLoaded: return "World not loaded";
        case Error::WorldInvalid: return "World is invalid";
        case Error::PlayerNotFound: return "Player not found";
        case Error::PlayerNotSpawned: return "Player not spawned";
        case Error::ActorNotFound: return "Actor not found";
        case Error::InvalidActorClass: return "Invalid actor class";
        case Error::SpawnFailed: return "Spawn failed";
        case Error::InvalidSpawnLocation: return "Invalid spawn location";
        case Error::InvalidOperation: return "Invalid operation";
        case Error::NullPointer: return "Null pointer";
        case Error::OutOfBounds: return "Out of bounds";
        case Error::HookFailed: return "Hook failed";
        case Error::HookNotFound: return "Hook not found";
        case Error::ConfigLoadFailed: return "Config load failed";
        case Error::ConfigSaveFailed: return "Config save failed";
        case Error::InvalidConfigValue: return "Invalid config value";
        default: return "Unknown error";
    }
}

} // namespace Broadsword
