#pragma once

#include <Windows.h>

namespace Broadsword {

/**
 * InputContext - Input state manager (stub for now)
 *
 * Provides access to keyboard, mouse, and gamepad state.
 * Will be fully implemented in future checkpoints.
 */
class InputContext {
public:
    InputContext() = default;

    /**
     * Check if key is currently down
     * @param vkey Virtual key code
     * @return true if key is down
     */
    bool IsKeyDown(int vkey) const {
        return (GetAsyncKeyState(vkey) & 0x8000) != 0;
    }

    /**
     * Check if key was just pressed this frame
     * @param vkey Virtual key code
     * @return true if key was just pressed
     */
    bool WasKeyPressed(int vkey) const {
        // Simplified for now
        return IsKeyDown(vkey);
    }
};

} // namespace Broadsword
