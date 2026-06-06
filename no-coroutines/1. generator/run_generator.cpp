#include "common/generator/Tree.h"
#include "common/io/print.h"
#include "no-coroutines/1. generator/Generator.h"
#include "no-coroutines/1. generator/TreeGenerator.h"

namespace no_coroutines {

Generator fibonacci(const int count) {
    return Generator(count);
}

void run_fibonacci_demo() {
    io::print("Fibonacci (10 terms):\n");
    auto generator = fibonacci(10);
    for (const int value : generator) {
        io::print("{}\n", value);
    }

    io::print("Second pass (exhausted):\n");
    for (const int value : generator) {
        io::print("{}\n", value);
    }
}

void run_tree_demo() {
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

void run_generator() {
    run_fibonacci_demo();
    run_tree_demo();
}

}
