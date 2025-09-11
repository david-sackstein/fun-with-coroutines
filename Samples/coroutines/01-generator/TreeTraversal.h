#pragma once

// Coroutine generators for binary tree traversal.
// Each function recursively yields values from a subtree using co_yield.
// Recursive calls are iterated with range-based for loops over sub-generators.

#include "coroutines/return_types/Generator.h"

struct TreeNode;

namespace coroutines {

// Yield values in pre-order: root, then left subtree, then right subtree.
Generator preorder(const TreeNode* node);
// Yield values in in-order: left subtree, then root, then right subtree.
Generator inorder(const TreeNode* node);

}
