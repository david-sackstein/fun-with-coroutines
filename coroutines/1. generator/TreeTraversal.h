#pragma once

#include "coroutines/1. generator/Generator.h"

struct TreeNode;

namespace coroutines {

Generator preorder(const TreeNode* node);
Generator inorder(const TreeNode* node);

}
