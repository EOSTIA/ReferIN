#pragma once
#include <bits/stdc++.h>
using namespace std;

// ReferralNode: one node in the N-ary referral tree.
// Each user can refer an arbitrary number of children (sub-users),
// so children are stored in a dynamic vector of owning pointers.

struct ReferralNode
{
    string userId;
    string name;
    vector<unique_ptr<ReferralNode>> children;
    ReferralNode *parent = nullptr; // non-owning back-pointer

    ReferralNode(string id, string n)
        : userId(move(id)), name(move(n)) {}
};

// ReferralNetwork: owns the tree root and exposes operations for
// adding referrals, removing users, and traversing the hierarchy.
// A HashMap (HashIndex.h) is used externally for O(1) lookups
// instead of walking the tree, which would be O(N).

class ReferralNetwork
{
public:
    explicit ReferralNetwork(const string &rootId, const string &rootName);

    // Adds a child referral under parentId. Returns raw pointer to new node
    // (or nullptr if parent not found). Caller supplies index map for lookup.
    ReferralNode *addReferral(ReferralNode *parentNode,
                              const string &userId,
                              const string &name);

    ReferralNode *getRoot() const { return root.get(); }

    // Depth-first traversal, invoking visit(node) for every node.
    void traverseDFS(const function<void(ReferralNode *)> &visit) const;

    // Breadth-first traversal (level order), useful for "downline by level".
    void traverseBFS(const function<void(ReferralNode *)> &visit) const;

    // Counts total downline size (all descendants) of a given node.
    size_t countDownline(ReferralNode *node) const;

    // Removes a node and its entire subtree from its parent.
    // Returns true on success.
    bool removeSubtree(ReferralNode *node);

private:
    unique_ptr<ReferralNode> root;
};
