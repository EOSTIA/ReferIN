// Empirical validation of the complexity claims made for this project:
//   - unordered_map (our HashIndex's backing structure): O(1) avg insert/lookup
//   - BTree<K,V>:                                             O(log N) insert/lookup/delete
//   - LRUCache<K,V>:                                          O(1) get/put, measured hit rate
//   - Naive linear scan:                                      O(N) lookup, for comparison
//
// Build (from project root, alongside include/ and src/):
//   g++ -std=c++17 -O2 -Iinclude benchmark/benchmark.cpp -o benchmark/benchmark
// Run:
//   ./benchmark/benchmark
// Output: printed table + results.csv (import into Excel/Sheets for your resume/report)

#include <bits/stdc++.h>
using namespace std;

#include "BTree.h"
#include "LRUCache.h"

// Generic timer: runs op() once, returns elapsed microseconds.

template <typename Func>
double time_op_us(Func &&op)
{
    auto start = chrono::high_resolution_clock::now();
    op();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double, micro>(end - start).count();
}

// Naive O(N) linear scan lookup over an unsorted vector of keys.
// Returns true if found; this is the "before self-balancing structure" baseline.
bool linearScanFind(const vector<int> &data, int key)
{
    for (int v : data)
    {
        if (v == key)
            return true;
    }
    return false;
}

struct Result
{
    int n;
    double btreeInsert, btreeLookupHit, btreeDelete;
    double hashInsert, hashLookupHit;
    double linearScanLookup;
    double cacheHitRateSim; // % from a separate cache-hit-rate simulation (not size-dependent, run once)
};

int main()
{
    const vector<int> sizes = {1000, 10000, 100000, 1000000};
    mt19937 rng(42);

    const int trials = 1000;  // per-N average over this many independent ops
    const int batchSize = 64; // batch work per timing sample to avoid 0.0000 us readings

    vector<Result> results;

    cout << fixed << setprecision(4);

    for (int n : sizes)
    {
        cout << "\n=== N = " << n << " ===\n";

        // Populate structures with n records: key = i, value = i*2
        BTree<int, int> btree(32); // minimum degree 32: realistic "wide" B-Tree node
        unordered_map<int, int> hashMap;
        hashMap.reserve(n * 2);
        vector<int> linearData;
        linearData.reserve(n);

        for (int i = 0; i < n; ++i)
        {
            btree.insert(i, i * 2);
            hashMap[i] = i * 2;
            linearData.push_back(i);
        }

        uniform_int_distribution<int> existingKeyDist(0, n - 1);

        // average INSERT time (new keys, avoids overwrite)
        double total_btree_insert = 0, total_hash_insert = 0;
        for (int i = 0; i < trials; ++i)
        {
            total_btree_insert += time_op_us([&]()
                                             {
                for (int j = 0; j < batchSize; ++j)
                {
                    int key = n + i * batchSize + j; // guaranteed-new key
                    btree.insert(key, key * 2);
                } });
            total_hash_insert += time_op_us([&]()
                                            {
                for (int j = 0; j < batchSize; ++j)
                {
                    int key = n + i * batchSize + j; // guaranteed-new key
                    hashMap[key] = key * 2;
                } });
        }
        double avg_btree_insert = total_btree_insert / (trials * batchSize);
        double avg_hash_insert = total_hash_insert / (trials * batchSize);

        // average LOOKUP time (guaranteed hit)
        double total_btree_lookup = 0, total_hash_lookup = 0, total_linear_lookup = 0;
        volatile int sink = 0; // prevents the optimizer from eliding "unused" lookup results
        for (int i = 0; i < trials; ++i)
        {
            total_btree_lookup += time_op_us([&]()
                                             {
                for (int j = 0; j < batchSize; ++j)
                {
                    int key = existingKeyDist(rng);
                    int outVal = 0;
                    btree.search(key, outVal);
                    sink = outVal;
                } });
            total_hash_lookup += time_op_us([&]()
                                            {
                for (int j = 0; j < batchSize; ++j)
                {
                    int key = existingKeyDist(rng);
                    auto it = hashMap.find(key);
                    sink = (it != hashMap.end()) ? it->second : -1;
                } });
            total_linear_lookup += time_op_us([&]()
                                              {
                for (int j = 0; j < batchSize; ++j)
                {
                    int key = existingKeyDist(rng);
                    volatile bool found = linearScanFind(linearData, key);
                    (void)found;
                } });
        }
        double avg_btree_lookup = total_btree_lookup / (trials * batchSize);
        double avg_hash_lookup = total_hash_lookup / (trials * batchSize);
        double avg_linear_lookup = total_linear_lookup / (trials * batchSize);

        // average DELETE time
        // Insert `trials` disposable extra keys first, then time deleting them,
        // so we don't shrink the "real" n-sized dataset mid-benchmark.
        vector<int> deleteKeys;
        deleteKeys.reserve(trials * batchSize);
        for (int i = 0; i < trials * batchSize; ++i)
        {
            int key = n + trials * batchSize + i; // separate range from the insert-benchmark keys above
            btree.insert(key, key * 2);
            deleteKeys.push_back(key);
        }
        double total_btree_delete = 0;
        for (int i = 0; i < trials; ++i)
        {
            total_btree_delete += time_op_us([&]()
                                             {
                for (int j = 0; j < batchSize; ++j)
                {
                    btree.remove(deleteKeys[i * batchSize + j]);
                } });
        }
        double avg_btree_delete = total_btree_delete / (trials * batchSize);

        cout << "  BTree   insert: " << avg_btree_insert << " us | "
             << "lookup(hit): " << avg_btree_lookup << " us | "
             << "delete: " << avg_btree_delete << " us\n";
        cout << "  HashMap insert: " << avg_hash_insert << " us | "
             << "lookup(hit): " << avg_hash_lookup << " us\n";
        cout << "  Linear scan lookup: " << avg_linear_lookup << " us  "
             << "(" << setprecision(1)
             << (avg_linear_lookup / avg_btree_lookup) << "x slower than BTree)"
             << setprecision(4) << "\n";

        results.push_back({n, avg_btree_insert, avg_btree_lookup, avg_btree_delete,
                           avg_hash_insert, avg_hash_lookup, avg_linear_lookup, 0.0});
    }

    // LRU Cache hit-rate simulation.
    // Models a realistic "cache-aside" access pattern: 80% of lookups
    // target a "hot set" of just 5% of keys (classic 80/20 skew seen in
    // real workloads), the rest are uniformly random "cold" keys.
    // On a miss, we simulate fetching from the B-Tree and warming the cache.

    cout << "\n=== LRU Cache Hit Rate Simulation ===\n";
    {
        const int datasetSize = 100000;
        const int hotSetSize = datasetSize / 20; // top 5% of keys are "hot"
        // Size the cache so it can realistically hold the hot set (a cache
        // much smaller than your working set will never show a good hit
        // rate, regardless of policy; this is sizing the cache the way
        // you'd size it in practice, not rigging the result).
        const int youngCap = hotSetSize / 4;
        const int oldCap = hotSetSize;
        const int accesses = 200000;

        BTree<int, int> backingStore(32);
        for (int i = 0; i < datasetSize; ++i)
            backingStore.insert(i, i * 2);

        LRUCache<int, int> cache(youngCap, oldCap);

        uniform_int_distribution<int> hotDist(0, hotSetSize - 1);
        uniform_int_distribution<int> coldDist(0, datasetSize - 1);
        uniform_real_distribution<double> coinFlip(0.0, 1.0);

        long hits = 0, misses = 0;
        for (int i = 0; i < accesses; ++i)
        {
            int key = (coinFlip(rng) < 0.8) ? hotDist(rng) : coldDist(rng);
            int cachedValue = 0;
            if (cache.get(key, cachedValue))
            {
                ++hits;
            }
            else
            {
                ++misses;
                int val;
                backingStore.search(key, val); // fall through to "disk"
                cache.put(key, val);           // warm the cache
            }
        }

        double hitRate = 100.0 * hits / (hits + misses);
        cout << "  Dataset size: " << datasetSize
             << " | Cache capacity (young+old): " << (youngCap + oldCap)
             << " (" << setprecision(2)
             << (100.0 * (youngCap + oldCap) / datasetSize) << "% of dataset)\n"
             << setprecision(4);
        cout << "  Access pattern: 80% hit a hot set of " << hotSetSize
             << " keys (" << (100.0 * hotSetSize / datasetSize) << "% of dataset), 20% uniform random\n";
        cout << "  Total accesses: " << accesses << " | Hits: " << hits << " | Misses: " << misses << "\n";
        cout << "  >>> Cache hit rate: " << setprecision(2) << hitRate << "%\n"
             << setprecision(4);

        if (!results.empty())
            results.back().cacheHitRateSim = hitRate; // stash on last row for CSV convenience
    }

    // Write CSV for easy import into a spreadsheet / resume appendix.

    ofstream csv("results.csv");
    csv << "N,btree_insert_us,btree_lookup_us,btree_delete_us,"
           "hashmap_insert_us,hashmap_lookup_us,linear_scan_lookup_us,speedup_btree_vs_linear\n";
    for (auto &r : results)
    {
        csv << r.n << ","
            << r.btreeInsert << "," << r.btreeLookupHit << "," << r.btreeDelete << ","
            << r.hashInsert << "," << r.hashLookupHit << "," << r.linearScanLookup << ","
            << (r.linearScanLookup / r.btreeLookupHit) << "\n";
    }
    csv.close();

    cout << "\nResults written to results.csv\n";
    return 0;
}