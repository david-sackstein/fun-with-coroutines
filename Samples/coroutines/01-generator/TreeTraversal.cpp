#include "common/generator/Tree.h"
#include "coroutines/01-generator/TreeTraversal.h"

namespace coroutines {

// Visit the root first. Then the left subtree. Then the right subtree.
Generator preorder(const TreeNode* node) {
    if (node == nullptr) {
        // Empty subtree. Nothing to yield.
        co_return;
    }

    // Visit the root before either subtree.
    co_yield node->value;

    for (const int left_value : preorder(node->left)) {
        // Forward each value produced by the left subtree.
        co_yield left_value;
    }

    for (const int right_value : preorder(node->right)) {
        // Forward each value produced by the right subtree.
        co_yield right_value;
    }
}

// Visit the left subtree first. Then visit the root. Then visit the right subtree.
Generator inorder(const TreeNode* node) {
    if (node == nullptr) {
        // Empty subtree. Nothing to yield.
        co_return;
    }

    for (const int left_value : inorder(node->left)) {
        // Forward each value produced by the left subtree.
        co_yield left_value;
    }

    // Visit the root after the left subtree and before the right subtree.
    co_yield node->value;

    for (const int right_value : inorder(node->right)) {
        // Forward each value produced by the right subtree.
        co_yield right_value;
    }
}

}
