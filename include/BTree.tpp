#pragma once
// Implementation file for BTree<Key,Value>. Included at the bottom of BTree.h.

// ---------------- SEARCH ----------------
template <typename Key, typename Value>
bool BTree<Key, Value>::search(const Key& key, Value& outValue) const {
    return searchNode(root.get(), key, outValue) != nullptr;
}

template <typename Key, typename Value>
typename BTree<Key, Value>::Node*
BTree<Key, Value>::searchNode(Node* node, const Key& key, Value& outValue) const {
    if (!node) return nullptr;
    size_t i = 0;
    while (i < node->keys.size() && key > node->keys[i]) ++i;

    if (i < node->keys.size() && key == node->keys[i]) {
        outValue = node->values[i];
        return node;
    }
    if (node->leaf) return nullptr;
    return searchNode(node->children[i].get(), key, outValue);
}

// ---------------- INSERT ----------------
template <typename Key, typename Value>
void BTree<Key, Value>::insert(const Key& key, const Value& value) {
    Node* r = root.get();
    if (static_cast<int>(r->keys.size()) == 2 * t - 1) {
        // Root is full: grow the tree upward by one level.
        auto newRoot = std::make_unique<Node>(false);
        Node* newRootPtr = newRoot.get();
        newRootPtr->children.push_back(std::move(root));
        root = std::move(newRoot);
        splitChild(root.get(), 0);
        insertNonFull(root.get(), key, value);
    } else {
        insertNonFull(r, key, value);
    }
}

template <typename Key, typename Value>
void BTree<Key, Value>::splitChild(Node* parent, int idx) {
    Node* fullChild = parent->children[idx].get();
    auto newChild = std::make_unique<Node>(fullChild->leaf);

    // Move the upper half of keys/values/children to newChild.
    newChild->keys.assign(fullChild->keys.begin() + t, fullChild->keys.end());
    newChild->values.assign(fullChild->values.begin() + t, fullChild->values.end());
    if (!fullChild->leaf) {
        for (auto it = fullChild->children.begin() + t; it != fullChild->children.end(); ++it)
            newChild->children.push_back(std::move(*it));
        fullChild->children.resize(t);
    }

    Key midKey = fullChild->keys[t - 1];
    Value midValue = fullChild->values[t - 1];

    fullChild->keys.resize(t - 1);
    fullChild->values.resize(t - 1);

    parent->children.insert(parent->children.begin() + idx + 1, std::move(newChild));
    parent->keys.insert(parent->keys.begin() + idx, midKey);
    parent->values.insert(parent->values.begin() + idx, midValue);
}

template <typename Key, typename Value>
void BTree<Key, Value>::insertNonFull(Node* node, const Key& key, const Value& value) {
    int i = static_cast<int>(node->keys.size()) - 1;
    if (node->leaf) {
        node->keys.push_back(Key());
        node->values.push_back(Value());
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            --i;
        }
        node->keys[i + 1] = key;
        node->values[i + 1] = value;
    } else {
        while (i >= 0 && key < node->keys[i]) --i;
        ++i;
        if (static_cast<int>(node->children[i]->keys.size()) == 2 * t - 1) {
            splitChild(node, i);
            if (key > node->keys[i]) ++i;
        }
        insertNonFull(node->children[i].get(), key, value);
    }
}

// ---------------- DELETE ----------------
template <typename Key, typename Value>
int BTree<Key, Value>::findKeyIndex(Node* node, const Key& key) const {
    int idx = 0;
    while (idx < static_cast<int>(node->keys.size()) && node->keys[idx] < key) ++idx;
    return idx;
}

template <typename Key, typename Value>
bool BTree<Key, Value>::remove(const Key& key) {
    Value dummy;
    if (!search(key, dummy)) return false;
    removeFromNode(root.get(), key);
    if (root->keys.empty() && !root->leaf) {
        // Shrink tree height when root becomes empty.
        root = std::move(root->children[0]);
    }
    return true;
}

template <typename Key, typename Value>
void BTree<Key, Value>::removeFromNode(Node* node, const Key& key) {
    int idx = findKeyIndex(node, key);

    if (idx < static_cast<int>(node->keys.size()) && node->keys[idx] == key) {
        if (node->leaf) removeFromLeaf(node, idx);
        else removeFromInternal(node, idx);
        return;
    }

    if (node->leaf) return; // key not found (shouldn't happen, checked earlier)

    bool lastChild = (idx == static_cast<int>(node->keys.size()));
    if (static_cast<int>(node->children[idx]->keys.size()) < t) fill(node, idx);

    // After fill(), idx's child may have merged; recompute bounds.
    if (lastChild && idx > static_cast<int>(node->keys.size()))
        removeFromNode(node->children[idx - 1].get(), key);
    else
        removeFromNode(node->children[idx].get(), key);
}

template <typename Key, typename Value>
void BTree<Key, Value>::removeFromLeaf(Node* node, int idx) {
    node->keys.erase(node->keys.begin() + idx);
    node->values.erase(node->values.begin() + idx);
}

template <typename Key, typename Value>
void BTree<Key, Value>::removeFromInternal(Node* node, int idx) {
    Key key = node->keys[idx];

    if (static_cast<int>(node->children[idx]->keys.size()) >= t) {
        Key pred = getPredecessor(node, idx);
        // value comes along with predecessor swap
        Value predVal;
        search(pred, predVal);
        node->keys[idx] = pred;
        node->values[idx] = predVal;
        removeFromNode(node->children[idx].get(), pred);
    } else if (static_cast<int>(node->children[idx + 1]->keys.size()) >= t) {
        Key succ = getSuccessor(node, idx);
        Value succVal;
        search(succ, succVal);
        node->keys[idx] = succ;
        node->values[idx] = succVal;
        removeFromNode(node->children[idx + 1].get(), succ);
    } else {
        merge(node, idx);
        removeFromNode(node->children[idx].get(), key);
    }
}

template <typename Key, typename Value>
Key BTree<Key, Value>::getPredecessor(Node* node, int idx) {
    Node* cur = node->children[idx].get();
    while (!cur->leaf) cur = cur->children.back().get();
    return cur->keys.back();
}

template <typename Key, typename Value>
Key BTree<Key, Value>::getSuccessor(Node* node, int idx) {
    Node* cur = node->children[idx + 1].get();
    while (!cur->leaf) cur = cur->children.front().get();
    return cur->keys.front();
}

template <typename Key, typename Value>
void BTree<Key, Value>::fill(Node* node, int idx) {
    if (idx != 0 && static_cast<int>(node->children[idx - 1]->keys.size()) >= t) {
        borrowFromPrev(node, idx);
    } else if (idx != static_cast<int>(node->keys.size()) &&
               static_cast<int>(node->children[idx + 1]->keys.size()) >= t) {
        borrowFromNext(node, idx);
    } else {
        if (idx != static_cast<int>(node->keys.size())) merge(node, idx);
        else merge(node, idx - 1);
    }
}

template <typename Key, typename Value>
void BTree<Key, Value>::borrowFromPrev(Node* node, int idx) {
    Node* child = node->children[idx].get();
    Node* sibling = node->children[idx - 1].get();

    child->keys.insert(child->keys.begin(), node->keys[idx - 1]);
    child->values.insert(child->values.begin(), node->values[idx - 1]);
    if (!child->leaf)
        child->children.insert(child->children.begin(), std::move(sibling->children.back()));

    node->keys[idx - 1] = sibling->keys.back();
    node->values[idx - 1] = sibling->values.back();

    sibling->keys.pop_back();
    sibling->values.pop_back();
    if (!sibling->leaf) sibling->children.pop_back();
}

template <typename Key, typename Value>
void BTree<Key, Value>::borrowFromNext(Node* node, int idx) {
    Node* child = node->children[idx].get();
    Node* sibling = node->children[idx + 1].get();

    child->keys.push_back(node->keys[idx]);
    child->values.push_back(node->values[idx]);
    if (!child->leaf)
        child->children.push_back(std::move(sibling->children.front()));

    node->keys[idx] = sibling->keys.front();
    node->values[idx] = sibling->values.front();

    sibling->keys.erase(sibling->keys.begin());
    sibling->values.erase(sibling->values.begin());
    if (!sibling->leaf) sibling->children.erase(sibling->children.begin());
}

template <typename Key, typename Value>
void BTree<Key, Value>::merge(Node* node, int idx) {
    Node* child = node->children[idx].get();
    Node* sibling = node->children[idx + 1].get();

    child->keys.push_back(node->keys[idx]);
    child->values.push_back(node->values[idx]);

    for (auto& k : sibling->keys) child->keys.push_back(k);
    for (auto& v : sibling->values) child->values.push_back(v);
    if (!child->leaf)
        for (auto& c : sibling->children) child->children.push_back(std::move(c));

    node->keys.erase(node->keys.begin() + idx);
    node->values.erase(node->values.begin() + idx);
    node->children.erase(node->children.begin() + idx + 1); // frees sibling
}

// ---------------- TRAVERSAL ----------------
template <typename Key, typename Value>
void BTree<Key, Value>::inorder(const std::function<void(const Key&, const Value&)>& visit) const {
    inorderNode(root.get(), visit);
}

template <typename Key, typename Value>
void BTree<Key, Value>::inorderNode(Node* node,
                                     const std::function<void(const Key&, const Value&)>& visit) const {
    if (!node) return;
    size_t i;
    for (i = 0; i < node->keys.size(); ++i) {
        if (!node->leaf) inorderNode(node->children[i].get(), visit);
        visit(node->keys[i], node->values[i]);
    }
    if (!node->leaf) inorderNode(node->children[i].get(), visit);
}
