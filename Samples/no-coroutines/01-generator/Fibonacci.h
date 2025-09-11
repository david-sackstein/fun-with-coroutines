#pragma once

// State-machine implementation of the Fibonacci sequence generator.
// Compare with the coroutines version: the same logic requires an explicit
// Phase enum and manual state variables instead of local variables in a coroutine.

#include "common/generator/Iterator.h"

namespace no_coroutines {

class Generator {
public:
    explicit Generator(const int count)
        : _remaining(count), _phase(count > 0 ? Phase::Emit : Phase::Finished) {}

    // Advance to the next value. Returns true if a value is ready, false when the sequence is done.
    bool next() {
        switch (_phase) {
        case Phase::Finished:
            // The sequence is exhausted.
            return false;

        case Phase::Emit:
            // Snapshot the current Fibonacci value and advance the pair.
            _current_value = _a;
            {
                const int next_a = _b;
                const int next_b = _a + _b;
                _a = next_a;
                _b = next_b;
            }
            if (--_remaining == 0) {
                // All requested values have been emitted.
                _phase = Phase::Finished;
            }
            return true;
        }
        return false;
    }

    // Return the value produced by the most recent next() call.
    [[nodiscard]] int get_current_value() const {
        return _current_value;
    }

private:
    enum class Phase {
        Emit,      // more values remain to be produced
        Finished   // all requested values have been produced
    };

    int _remaining;     // how many more values to emit
    Phase _phase;
    int _current_value = 0;
    int _a = 0;         // current Fibonacci number
    int _b = 1;         // next Fibonacci number
};

// Support range-based for loops over a Generator.
inline GeneratorIterator<Generator> begin(Generator &generator) {
    return GeneratorIterator{generator};
}
inline GeneratorIterator<Generator> end(const Generator &) {
    return {};
}

// Construct a Generator for the first count Fibonacci numbers.
inline Generator make_fibonacci(const int count) {
    return Generator(count);
}

}
