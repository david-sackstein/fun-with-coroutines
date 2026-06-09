#pragma once

#include "coroutines/return_types/Generator.h"

struct TreeNode;

namespace coroutines {

Generator preorder(const TreeNode* node);
Generator inorder(const TreeNode* node);

}
