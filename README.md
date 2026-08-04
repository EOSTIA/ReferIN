A C++ project for a referral connection based on an N-ary Tree relation with a hashmap-based search lookup, and a B-Tree for managing databases and file systems with a doubly linked list-based LRU caching for a young and old list.

Build : 'g++ -std=c++17 -Iinclude src/*.cpp -o referral_system'

testing : g++ -std=c++17 -O2 -Iinclude benchmark/benchmark.cpp -o benchmark/benchmark