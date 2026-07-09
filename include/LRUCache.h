#pragma once
#include <list>
#include <unordered_map>
// #include <optional> works only gcc compilers version from 7 and old
using namespace std;

// LRUCache<Key, Value>: a two-generation ("young"/"old") LRU cache.
//
// Why two lists instead of one?
// A plain LRU cache is vulnerable to "cache pollution": one big
// sequential scan (e.g. printing every record once) shoves every
// truly "hot" item out of the cache, even though the scanned items
// will never be touched again.
//
// Fix: new/rarely-seen items land in the YOUNG generation. Only
// once an item is accessed again (proving it's actually hot) does
// it get PROMOTED into the OLD generation. Eviction always happens
// from the young generation first, protecting proven-hot data in
// the old generation from one-off scans.
//
// Both generations are (list + hash map) pairs, same idea as a
// textbook single-list LRU: the list keeps recency order (front =
// most-recent), the map gives O(1) access to a list iterator.

template <typename Key, typename Value>
class LRUCache
{
public:
    LRUCache(size_t youngCapacity, size_t oldCapacity)
        : youngCap(youngCapacity), oldCap(oldCapacity) {}

    // Returns the value if present, promoting it appropriately.
    bool get(const Key &key, Value &value)
    {
        auto oldIt = oldMap.find(key);
        if (oldIt != oldMap.end())
        {
            // Already in old generation
            oldList.splice(oldList.begin(), oldList, oldIt->second);
            value = oldIt->second->second;
            return true;
        }

        auto youngIt = youngMap.find(key);
        if (youngIt != youngMap.end())
        {
            // Promote to old generation
            value = youngIt->second->second;
            youngList.erase(youngIt->second);
            youngMap.erase(youngIt);

            insertOld(key, value);
            return true;
        }

        return false;
    }

    // Inserts/updates a key. New keys start in the young generation.
    void put(const Key &key, const Value &value)
    {
        if (oldMap.count(key))
        {
            oldMap[key]->second = value;
            oldList.splice(oldList.begin(), oldList, oldMap[key]);
            return;
        }
        if (youngMap.count(key))
        {
            youngMap[key]->second = value;
            youngList.splice(youngList.begin(), youngList, youngMap[key]);
            return;
        }
        insertYoung(key, value);
    }

    size_t youngSize() const { return youngList.size(); }
    size_t oldSize() const { return oldList.size(); }

private:
    using ListType = list<pair<Key, Value>>;
    using MapType = unordered_map<Key, typename ListType::iterator>;

    size_t youngCap, oldCap;
    ListType youngList, oldList;
    MapType youngMap, oldMap;

    void insertYoung(const Key &key, const Value &value)
    {
        youngList.emplace_front(key, value);
        youngMap[key] = youngList.begin();
        if (youngList.size() > youngCap)
            evictYoung();
    }

    void insertOld(const Key &key, const Value &value)
    {
        oldList.emplace_front(key, value);
        oldMap[key] = oldList.begin();
        if (oldList.size() > oldCap)
            evictOld();
    }

    void evictYoung()
    {
        auto &last = youngList.back();
        youngMap.erase(last.first);
        youngList.pop_back();
    }

    void evictOld()
    {
        auto &last = oldList.back();
        oldMap.erase(last.first);
        oldList.pop_back();
    }
};
