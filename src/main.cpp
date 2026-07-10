#include <iostream>
#include <iomanip>
#include "ReferralNetwork.h"
#include "HashIndex.h"
#include "BTree.h"
#include "LRUCache.h"
#include "CsvLoader.h"
using namespace std;

// System glues the four structures together the way a real referral
// platform would: tree = source of truth for hierarchy, hash index =
// fast pointer lookup, B-Tree = durable sorted storage (keyed by
// phone number), LRU cache = hot-path read acceleration in front of
// the B-Tree. UserRecord (full CSV row) is defined in CsvLoader.h.

class ReferralSystem
{
public:
    ReferralSystem(const UserRecord &rootRecord)
        : network(rootRecord.phone, rootRecord.name), cache(/*young*/ 5, /*old*/ 5)
    {
        ReferralNode *root = network.getRoot();
        index.insert(rootRecord.phone, root);
        btree.insert(rootRecord.phone, rootRecord);
    }

    bool refer(const string &referrerPhone, const UserRecord &rec)
    {
        ReferralNode *parent = index.find(referrerPhone);
        if (!parent)
        {
            cout << "  [error] referrer " << referrerPhone << " not found\n";
            return false;
        }
        ReferralNode *child = network.addReferral(parent, rec.phone, rec.name);
        index.insert(rec.phone, child);
        btree.insert(rec.phone, rec);
        return true;
    }

    // Read path: cache first, fall back to B-Tree ("disk") on miss.
    void lookup(const string &phone)
{
    UserRecord cached;

    if (cache.get(phone, cached))
    {
        cout << "  [cache hit]  " << phone << " -> "
                  << cached.name
                  << " (" << cached.job << ", "
                  << cached.city << ")\n";
        return;
    }

    UserRecord record;
    if (btree.search(phone, record))
    {
        cout << "  [cache miss -> btree hit] " << phone
                  << " -> " << record.name
                  << " (" << record.job << ", "
                  << record.city << ")\n";

        cache.put(phone, record);   // Warm the cache
    }
    else
    {
        cout << "  [not found] " << phone << "\n";
    }
}

    void printDownlineCount(const string &phone)
    {
        ReferralNode *node = index.find(phone);
        if (!node)
        {
            cout << "  [error] " << phone << " not found\n";
            return;
        }
        cout << "  " << node->name << " has " << network.countDownline(node)
                  << " total downline members\n";
    }

    void printTreeSample(size_t maxNodes)
    {
        size_t count = 0;
        network.traverseBFS([&](ReferralNode *n)
                            {
            if (count++ >= maxNodes) return;
            cout << "  " << n->userId << " (" << n->name << ")"
                       << (n->parent ? "  <- referred by " + n->parent->userId : "  <- root") << "\n"; });
    }

    void printSortedStorageSample(size_t maxRows)
    {
        size_t count = 0;
        btree.inorder([&](const string &phone, const UserRecord &rec)
                      {
            if (count++ >= maxRows) return;
            cout << "  " << phone << " | " << setw(20) << left << rec.name
                       << " | " << rec.city << " | followers=" << rec.followers << "\n"; });
    }

    size_t userCount() const { return index.size(); }

private:
    ReferralNetwork network;
    HashIndex index;
    BTree<string, UserRecord> btree;
    LRUCache<string, UserRecord> cache;
};

int main()
{
    auto users = CsvLoader::load("D:/ReferIN - C++/referral_system/data/generated_data.csv");
    if (users.empty())
    {
        cerr << "Could not load data\n";
        return 1;
    }
    cout << "Loaded " << users.size() << " users from CSV\n\n";

    // ---- Build referral network from the dataset ----
    // No referrer column exists in the raw data, so we synthesize a
    // realistic multi-level hierarchy: each user (after the first) is
    // referred by an earlier user, branching ~3 ways per referrer.
    // This is the only "manual" part — everything else (name, email,
    // city, age, job, followers) comes straight from the CSV row.
    ReferralSystem system(users[0]);
    for (size_t i = 1; i < users.size(); ++i)
    {
        const string &parentPhone = users[(i - 1) / 3].phone;
        system.refer(parentPhone, users[i]);
    }

    cout << "== Referral tree sample (first 10 nodes, BFS) ==\n";
    system.printTreeSample(10);

    cout << "\n== Downline counts ==\n";
    system.printDownlineCount(users[0].phone);
    system.printDownlineCount(users[1].phone);

    cout << "\n== Sorted storage sample, B-Tree inorder by phone (first 10) ==\n";
    system.printSortedStorageSample(10);

    cout << "\n== Cache-accelerated lookups ==\n";
    system.lookup(users[5].phone);   // cold -> btree hit, warms cache
    system.lookup(users[5].phone);   // now a cache hit
    system.lookup("+91-0000000000"); // does not exist

    cout << "\nTotal indexed users: " << system.userCount() << "\n";
    return 0;
}
