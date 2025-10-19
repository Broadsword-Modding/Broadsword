#pragma once

#include "Error.hpp"
#include <variant>
#include <utility>

namespace Broadsword {

/**
 * Result monad for type-safe error handling
 *
 * Represents either a successful value (T) or an error (E).
 * Eliminates need for exceptions and makes error handling explicit.
 *
 * Usage:
 *   Result<Player*> GetPlayer() {
 *       if (!world) return Error::WorldNotLoaded;
 *       auto* player = FindPlayer();
 *       if (!player) return Error::PlayerNotFound;
 *       return player;
 *   }
 *
 *   auto result = GetPlayer();
 *   if (result) {
 *       auto* player = result.Value();
 *       // use player
 *   } else {
 *       log.Error("Failed: {}", ToString(result.GetError()));
 *   }
 *
 * C++26 Pattern Matching (future):
 *   inspect (result) {
 *       <Ok> player => { ... }
 *       <Error::PlayerNotFound> => { ... }
 *       _ => { ... }
 *   }
 */
template<typename T, typename E = Error>
class Result {
public:
    /**
     * Construct a successful Result with a value
     */
    Result(T value) : m_Value(std::move(value)) {}

    /**
     * Construct a failed Result with an error
     */
    Result(E error) : m_Value(std::move(error)) {}

    /**
     * Check if the Result contains a value (success)
     */
    bool IsOk() const {
        return std::holds_alternative<T>(m_Value);
    }

    /**
     * Check if the Result contains an error (failure)
     */
    bool IsError() const {
        return std::holds_alternative<E>(m_Value);
    }

    /**
     * Get the value (only call if IsOk() returns true)
     * Undefined behavior if called on an error Result
     */
    T& Value() {
        return std::get<T>(m_Value);
    }

    /**
     * Get the value (const version)
     */
    const T& Value() const {
        return std::get<T>(m_Value);
    }

    /**
     * Get the error (only call if IsError() returns true)
     * Undefined behavior if called on a success Result
     */
    E& GetError() {
        return std::get<E>(m_Value);
    }

    /**
     * Get the error (const version)
     */
    const E& GetError() const {
        return std::get<E>(m_Value);
    }

    /**
     * Get value or return default if error
     */
    T ValueOr(T defaultValue) const {
        if (IsOk()) {
            return std::get<T>(m_Value);
        }
        return defaultValue;
    }

    /**
     * Allow using Result in boolean context
     * if (result) { ... } is equivalent to if (result.IsOk()) { ... }
     */
    explicit operator bool() const {
        return IsOk();
    }

    /**
     * Map: Transform the value if Ok, otherwise propagate error
     *
     * Usage:
     *   Result<int> r = GetNumber();
     *   Result<int> doubled = r.Map([](int x) { return x * 2; });
     */
    template<typename F>
    auto Map(F&& func) const -> Result<decltype(func(std::declval<T>())), E> {
        if (IsOk()) {
            return func(Value());
        }
        return GetError();
    }

    /**
     * AndThen: Chain operations that return Result
     *
     * Usage:
     *   Result<Player*> player = GetPlayer()
     *       .AndThen([](Player* p) { return GetHealth(p); })
     *       .AndThen([](float health) { return ValidateHealth(health); });
     */
    template<typename F>
    auto AndThen(F&& func) const -> decltype(func(std::declval<T>())) {
        if (IsOk()) {
            return func(Value());
        }
        using ReturnType = decltype(func(std::declval<T>()));
        return ReturnType(GetError());
    }

private:
    std::variant<T, E> m_Value;
};

} // namespace Broadsword
