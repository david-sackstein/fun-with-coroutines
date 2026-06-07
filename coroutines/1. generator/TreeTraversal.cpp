#include "common/generator/Tree.h"
#include "coroutines/1. generator/TreeTraversal.h"

namespace coroutines {

Generator preorder(const TreeNode* node) {
    if (node == nullptr) {
        co_return;
    }

    co_yield node->value;

    for (const int left_value : preorder(node->left)) {
        co_yield left_value;
    }

    for (const int right_value : preorder(node->right)) {
        co_yield right_value;
    }
}

Generator inorder(const TreeNode* node) {
    if (node == nullptr) {
        co_return;
    }

    for (const int left_value : inorder(node->left)) {
        co_yield left_value;
    }

    co_yield node->value;

    for (const int right_value : inorder(node->right)) {
        co_yield right_value;
    }
}

}
