#pragma once

#include <iterator>

// Generic iterator for any Generator type that implements:
// - bool next()
// - int get_current_value()
template<typename GeneratorType>
class GeneratorIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int*;
    using reference = const int&;

    // End iterator constructor
    GeneratorIterator() : _generator(nullptr), _done(true) {}

    // Begin iterator constructor
    explicit GeneratorIterator(GeneratorType& generator) 
        : _generator(&generator), _done(false) {
        // Advance to first element
        advance();
    }

    reference operator*() const {
        return _current_value;
    }

    pointer operator->() const {
        return &_current_value;
    }

    GeneratorIterator& operator++() {
        advance();
        return *this;
    }

    GeneratorIterator operator++(int) {
        GeneratorIterator tmp = *this;
        advance();
        return tmp;
    }

    bool operator==(const GeneratorIterator& other) const {
        // Two end iterators are equal
        if (_done && other._done) return true;
        // Begin and end are not equal
        if (_done != other._done) return false;
        // Two begin iterators from same generator are equal
        return _generator == other._generator;
    }

    bool operator!=(const GeneratorIterator& other) const {
        return !(this->operator==(other));
    }

private:
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

// Free functions for range-based for loop support
template<typename GeneratorType>
inline GeneratorIterator<GeneratorType> begin(GeneratorType& generator) {
    return GeneratorIterator<GeneratorType>(generator);
}

template<typename GeneratorType>
inline GeneratorIterator<GeneratorType> end(GeneratorType& generator) {
    return GeneratorIterator<GeneratorType>();
}

