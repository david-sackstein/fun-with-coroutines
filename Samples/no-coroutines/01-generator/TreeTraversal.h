#pragma once

// Explicit-stack implementations of tree traversal generators.
// Compare with the coroutines version: the recursive structure of the traversal
// is replaced by a manually managed stack.

#include "common/generator/Iterator.h"
#include "common/generator/Tree.h"

#include <vector>

namespace no_coroutines {

// Iterates a binary tree in pre-order (root, left, right) using an explicit stack.
// Nodes are pushed right-before-left so that left is popped first.
class PreorderTreeGenerator {
public:
    explicit PreorderTreeGenerator(const TreeNode* root) {
        if (root != nullptr) {
            _stack.push_back(root);
        }
    }

    // Pop the next node, record its value, and push its children right-then-left.
    // Returns true if a value is ready, false when all nodes have been visited.
    bool next() {
        if (_stack.empty()) {
            // All nodes have been visited.
            return false;
        }

        const TreeNode* const node = _stack.back();
        _stack.pop_back();
        _current_value = node->value;

        // Push right first so left is on top and processed next.
        if (node->right != nullptr) {
            _stack.push_back(node->right);
        }

        if (node->left != nullptr) {
            _stack.push_back(node->left);
        }

        return true;
    }

    // Return the value produced by the most recent next() call.
    [[nodiscard]] int get_current_value() const {
        return _current_value;
    }

private:
    // Pending nodes to visit; back of vector is the next node to process.
    std::vector<const TreeNode*> _stack;
    int _current_value = 0;
};

// Iterates a binary tree in in-order (left, root, right) using an explicit stack.
// Descends to the leftmost node first, then unwinds through ancestors.
class InorderTreeGenerator {
public:
    explicit InorderTreeGenerator(const TreeNode* root) : _current(root) {}

    // Descend to the leftmost unvisited node, visit it, then move to its right subtree.
    // Returns true if a value is ready, false when all nodes have been visited.
    bool next() {
        // Push all nodes along the left spine of the current subtree.
        while (_current != nullptr) {
            _stack.push_back(_current);
            _current = _current->left;
        }

        if (_stack.empty()) {
            // All nodes have been visited.
            return false;
        }

        // Pop and visit the next node in in-order sequence.
        _current = _stack.back();
        _stack.pop_back();
        _current_value = _current->value;
        // Continue from the right subtree on the next call.
        _current = _current->right;
        return true;
    }

    // Return the value produced by the most recent next() call.
    [[nodiscard]] int get_current_value() const {
        return _current_value;
    }

private:
    // Next node to descend into; nullptr when the current spine is exhausted.
    const TreeNode* _current = nullptr;

    // Ancestors waiting to be visited after their left subtrees are done.
    std::vector<const TreeNode*> _stack;
    int _current_value = 0;
};

// Support range-based for loops over PreorderTreeGenerator.
inline GeneratorIterator<PreorderTreeGenerator> begin(PreorderTreeGenerator& generator) {
    return GeneratorIterator(generator);
}
inline GeneratorIterator<PreorderTreeGenerator> end(const PreorderTreeGenerator&) {
    return {};
}

// Support range-based for loops over InorderTreeGenerator.
inline GeneratorIterator<InorderTreeGenerator> begin(InorderTreeGenerator& generator) {
    return GeneratorIterator(generator);
}
inline GeneratorIterator<InorderTreeGenerator> end(const InorderTreeGenerator&) {
    return {};
}

}
