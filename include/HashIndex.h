#pragma once
#include <string>
#include <unordered_map>
#include "ReferralNetwork.h"
using namespace std;

// HashIndex: maps userId -> raw pointer to the corresponding
// ReferralNode. This turns "find user X in the tree" from an
// O(N) DFS/BFS scan into an O(1) average-time hash lookup.
//
// IMPORTANT: This class does NOT own the nodes. The tree
// (ReferralNetwork) owns them via unique_ptr. If a node is
// removed from the tree, you must also erase it (and all its
// descendants) from this index, or you'll have a dangling pointer.

class HashIndex {
public:
    void insert(const string& userId, ReferralNode* node) {
        index_[userId] = node;
    }

    ReferralNode* find(const string& userId) const {
        auto it = index_.find(userId);
        return it == index_.end() ? nullptr : it->second;
    }

    bool contains(const string& userId) const {
        return index_.find(userId) != index_.end();
    }

    // Removes a single id from the index (does not touch descendants).
    void erase(const string& userId) {
        index_.erase(userId);
    }

    // Removes an entire subtree's ids from the index. Call this BEFORE
    // ReferralNetwork::removeSubtree destroys the nodes, since we need
    // to walk them first to know which ids to drop.
    void eraseSubtree(ReferralNode* node) {
        if (!node) return;
        index_.erase(node->userId);
        for (auto& c : node->children) eraseSubtree(c.get());
    }

    size_t size() const { return index_.size(); }

private:
    unordered_map<string, ReferralNode*> index_;
};
