#pragma once

#include "common/generator/Iterator.h"
#include "common/generator/Tree.h"

#include <vector>

namespace no_coroutines {

class PreorderTreeGenerator {
public:
    explicit PreorderTreeGenerator(const TreeNode* root) {
        if (root != nullptr) {
            _stack.push_back(root);
        }
    }

    bool next() {
        if (_stack.empty()) {
            return false;
        }

        const TreeNode* const node = _stack.back();
        _stack.pop_back();
        _current_value = node->value;

        if (node->right != nullptr) {
            _stack.push_back(node->right);
        }
        if (node->left != nullptr) {
            _stack.push_back(node->left);
        }

        return true;
    }

    [[nodiscard]] int get_current_value() const {
        return _current_value;
    }

private:
    std::vector<const TreeNode*> _stack;
    int _current_value = 0;
};

class InorderTreeGenerator {
public:
    explicit InorderTreeGenerator(const TreeNode* root) : _current(root) {}

    bool next() {
        while (_current != nullptr) {
            _stack.push_back(_current);
            _current = _current->left;
        }

        if (_stack.empty()) {
            return false;
        }

        _current = _stack.back();
        _stack.pop_back();
        _current_value = _current->value;
        _current = _current->right;
        return true;
    }

    [[nodiscard]] int get_current_value() const {
        return _current_value;
    }

private:
    const TreeNode* _current = nullptr;
    std::vector<const TreeNode*> _stack;
    int _current_value = 0;
};

inline GeneratorIterator<PreorderTreeGenerator> begin(PreorderTreeGenerator& generator) {
    return GeneratorIterator(generator);
}

inline GeneratorIterator<PreorderTreeGenerator> end(const PreorderTreeGenerator&) {
    return {};
}

inline GeneratorIterator<InorderTreeGenerator> begin(InorderTreeGenerator& generator) {
    return GeneratorIterator(generator);
}

inline GeneratorIterator<InorderTreeGenerator> end(const InorderTreeGenerator&) {
    return {};
}

}
