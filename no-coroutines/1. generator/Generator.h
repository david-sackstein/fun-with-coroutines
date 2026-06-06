#pragma once

#include "common/generator/Iterator.h"

namespace no_coroutines {
    class Generator {
    public:
        explicit Generator(const int count)
            : _remaining(count), _phase(count > 0 ? Phase::Emit : Phase::Finished) {}

        bool next() {
            switch (_phase) {
            case Phase::Finished:
                return false;

            case Phase::Emit:
                _current_value = _a;
                {
                    const int next_a = _b;
                    const int next_b = _a + _b;
                    _a = next_a;
                    _b = next_b;
                }
                if (--_remaining == 0) {
                    _phase = Phase::Finished;
                }
                return true;
            }
            return false;
        }

        [[nodiscard]] int get_current_value() const {
            return _current_value;
        }

    private:
        enum class Phase {
            Emit,
            Finished
        };

        int _remaining;
        Phase _phase;
        int _current_value = 0;
        int _a = 0;
        int _b = 1;
    };

    inline GeneratorIterator<Generator> begin(Generator& generator) {
        return GeneratorIterator(generator);
    }

    inline GeneratorIterator<Generator> end(const Generator&) {
        return {};
    }
}
