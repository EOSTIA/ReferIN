#include "ReferralNetwork.h"
#include <queue>
#include <algorithm>

ReferralNetwork::ReferralNetwork(const std::string &rootId, const std::string &rootName)
{
    root = std::make_unique<ReferralNode>(rootId, rootName);
}

ReferralNode *ReferralNetwork::addReferral(ReferralNode *parentNode,
                                           const std::string &userId,
                                           const std::string &name)
{
    if (!parentNode)
        return nullptr;
    auto child = std::make_unique<ReferralNode>(userId, name);
    child->parent = parentNode;
    ReferralNode *rawPtr = child.get();
    parentNode->children.push_back(std::move(child));
    return rawPtr;
}

void ReferralNetwork::traverseDFS(const std::function<void(ReferralNode *)> &visit) const
{
    std::function<void(ReferralNode *)> dfs = [&](ReferralNode *node)
    {
        if (!node)
            return;
        visit(node);
        for (auto &c : node->children)
            dfs(c.get());
    };
    dfs(root.get());
}

void ReferralNetwork::traverseBFS(const std::function<void(ReferralNode *)> &visit) const
{
    std::queue<ReferralNode *> q;
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
    std::function<void(ReferralNode *)> dfs = [&](ReferralNode *n)
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
    auto it = std::find_if(siblings.begin(), siblings.end(),
                           [&](const std::unique_ptr<ReferralNode> &p)
                           { return p.get() == node; });
    if (it == siblings.end())
        return false;
    siblings.erase(it); // unique_ptr destructor recursively frees subtree
    return true;
}
