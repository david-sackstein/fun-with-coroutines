#pragma once

#include <print>

namespace no_coroutines {
    class Generator {
    public:
        Generator(int count) : _count(count), _current_index(0), _current_value(0) {}

        bool next() {
            if (_current_index >= _count) {
                return false;
            }
            _current_value = _current_index;
            _current_index++;
            return true;
        }

        int get_current_value() const {
            return _current_value;
        }

    private:
        int _count;
        int _current_index;
        int _current_value;
    };
}
