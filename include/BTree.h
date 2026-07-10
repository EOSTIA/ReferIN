#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <functional>

// ---------------------------------------------------------------
// BTree<Key, Value>: a classic disk-oriented B-Tree of minimum
// degree t (each node holds between t-1 and 2t-1 keys, except the
// root). Simulates how a real database/file-system index keeps
// data sorted on "disk blocks" with O(log N) search/insert/delete
// and self-balancing via split/merge instead of rotations.
// ---------------------------------------------------------------
template <typename Key, typename Value>
class BTree {
public:
    explicit BTree(int minDegree = 3) : t(minDegree) {
        root = std::make_unique<Node>(true);
    }

    void insert(const Key& key, const Value& value);
    bool search(const Key& key, Value& outValue) const;
    bool remove(const Key& key);
    void inorder(const std::function<void(const Key&, const Value&)>& visit) const;

private:
    struct Node {
        bool leaf;
        std::vector<Key> keys;
        std::vector<Value> values;
        std::vector<std::unique_ptr<Node>> children; // size = keys.size()+1 if internal

        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    int t; // minimum degree
    std::unique_ptr<Node> root;

    Node* searchNode(Node* node, const Key& key, Value& outValue) const;
    void splitChild(Node* parent, int idx);
    void insertNonFull(Node* node, const Key& key, const Value& value);

    void removeFromNode(Node* node, const Key& key);
    int findKeyIndex(Node* node, const Key& key) const;
    void removeFromLeaf(Node* node, int idx);
    void removeFromInternal(Node* node, int idx);
    Key getPredecessor(Node* node, int idx);
    Key getSuccessor(Node* node, int idx);
    void fill(Node* node, int idx);
    void borrowFromPrev(Node* node, int idx);
    void borrowFromNext(Node* node, int idx);
    void merge(Node* node, int idx);

    void inorderNode(Node* node, const std::function<void(const Key&, const Value&)>& visit) const;
};

#include "BTree.tpp"
