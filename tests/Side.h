#pragma once

enum class Side {
    Coroutines,
    NoCoroutines,
};

inline const char *side_name(const Side side) {
    return side == Side::Coroutines ? "Coroutines" : "NoCoroutines";
}
