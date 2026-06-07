#include "common/generator/Tree.h"
#include "common/io/print.h"
#include "no-coroutines/1. generator/TreeGenerator.h"

namespace no_coroutines {

void run_tree_sample() {
    const TreeNode* const root = sample_tree();

    io::print("\nBST pre-order DFS:\n");
    PreorderTreeGenerator preorder(root);
    for (const int value : preorder) {
        io::print("{}\n", value);
    }

    io::print("\nBST in-order DFS:\n");
    InorderTreeGenerator inorder(root);
    for (const int value : inorder) {
        io::print("{}\n", value);
    }
}

}
