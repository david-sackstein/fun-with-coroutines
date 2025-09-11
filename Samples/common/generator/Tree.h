#pragma once

// A binary tree node holding an integer value and optional left/right children.
struct TreeNode {
    int value;
    const TreeNode* left;
    const TreeNode* right;
};

// Return the root of a sample binary search tree (left < node < right):
//
//           4
//          ╱ ╲
//         2   6
//        ╱ ╲ ╱ ╲
//       1  3 5  7
//
// Pre-order traversal:  4, 2, 1, 3, 6, 5, 7
// In-order traversal:   1, 2, 3, 4, 5, 6, 7
inline const TreeNode* sample_tree() {
    static constexpr TreeNode n1{1, nullptr, nullptr};
    static constexpr TreeNode n3{3, nullptr, nullptr};
    static constexpr TreeNode n5{5, nullptr, nullptr};
    static constexpr TreeNode n7{7, nullptr, nullptr};
    static constexpr TreeNode n2{2, &n1, &n3};
    static constexpr TreeNode n6{6, &n5, &n7};
    static constexpr TreeNode n4{4, &n2, &n6};
    return &n4;
}
