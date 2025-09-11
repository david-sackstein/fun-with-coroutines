#pragma once

// Input iterator adapter for any Generator type that implements:
//   bool next()              — advance and return true if a value is ready
//   int get_current_value()  — return the value produced by the last next()
//
// Enables range-based for loops over generators.
// Construction with a generator immediately calls next() to reach the first value.

#include <iterator>

template<typename GeneratorType>
class GeneratorIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int*;
    using reference = const int&;

    // Construct the end-of-sequence sentinel.
    GeneratorIterator() : _generator(nullptr), _done(true) {}

    // Construct a begin iterator and advance to the first value.
    explicit GeneratorIterator(GeneratorType& generator)
        : _generator(&generator), _done(false) {
        advance();
    }

    reference operator*() const {
        return _current_value;
    }

    pointer operator->() const {
        return &_current_value;
    }

    // Advance to the next value.
    GeneratorIterator& operator++() {
        advance();
        return *this;
    }

    // NOLINTNEXTLINE(readability-const-return-type) -- cert-dcl21-cpp requires const postfix operator++ result
    const GeneratorIterator operator++(int) {
        GeneratorIterator tmp = *this;
        advance();
        return tmp;
    }

    bool operator==(const GeneratorIterator& other) const {
        if (_done && other._done) {
            // Both are end sentinels.
            return true;
        }
        if (_done != other._done) {
            // One is at end, the other is not.
            return false;
        }
        // Both are active iterators; equal only if they point to the same generator.
        return _generator == other._generator;
    }

    bool operator!=(const GeneratorIterator& other) const {
        return !(this->operator==(other));
    }

private:
    // Call next() on the generator and update the current value, or mark done.
    void advance() {
        if (!_generator || !_generator->next()) {
            _done = true;
        } else {
            _current_value = _generator->get_current_value();
        }
    }

    GeneratorType* _generator;
    bool _done;
    int _current_value = 0;
};

