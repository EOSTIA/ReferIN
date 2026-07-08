#include "ReferralNetwork.h"
#include <bits/stdc++.h>
using namespace std;

ReferralNetwork::ReferralNetwork(const string &rootId, const string &rootName)
{
    root = make_unique<ReferralNode>(rootId, rootName);
}

ReferralNode *ReferralNetwork::addReferral(ReferralNode *parentNode,
                                           const string &userId,
                                           const string &name)
{
    if (!parentNode)
        return nullptr;
    auto child = make_unique<ReferralNode>(userId, name);
    child->parent = parentNode;
    ReferralNode *rawPtr = child.get();
    parentNode->children.push_back(move(child));
    return rawPtr;
}

void ReferralNetwork::traverseDFS(const function<void(ReferralNode *)> &visit) const
{
    function<void(ReferralNode *)> dfs = [&](ReferralNode *node)
    {
        if (!node)
            return;
        visit(node);
        for (auto &c : node->children)
            dfs(c.get());
    };
    dfs(root.get());
}

void ReferralNetwork::traverseBFS(const function<void(ReferralNode *)> &visit) const
{
    queue<ReferralNode *> q;
    q.push(root.get());
    while (!q.empty())
    {
        ReferralNode *cur = q.front();
        q.pop();
        visit(cur);
        for (auto &c : cur->children)
            q.push(c.get());
    }
}

size_t ReferralNetwork::countDownline(ReferralNode *node) const
{
    if (!node)
        return 0;
    size_t count = 0;
    function<void(ReferralNode *)> dfs = [&](ReferralNode *n)
    {
        for (auto &c : n->children)
        {
            ++count;
            dfs(c.get());
        }
    };
    dfs(node);
    return count;
}

bool ReferralNetwork::removeSubtree(ReferralNode *node)
{
    if (!node || !node->parent)
        return false; // cannot remove root this way
    auto &siblings = node->parent->children;
    auto it = find_if(siblings.begin(), siblings.end(),
                      [&](const unique_ptr<ReferralNode> &p)
                      { return p.get() == node; });
    if (it == siblings.end())
        return false;
    siblings.erase(it); // unique_ptr destructor recursively frees subtree
    return true;
}
