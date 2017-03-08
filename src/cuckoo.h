#include <vector>
using namespace std;

class cuckoo_hash_set {
private:
    vector<int> ht;
    size_t mask;
    size_t hash_1(int v) {
        return (0xbabadeadLLU * v + 0xdeadbaba) & mask;
    }
    size_t hash_2(int v) {
        return (131LLU * v + 31) & mask;
    }
public:
    cuckoo_hash_set() {
        mask = 1;
        ht.resize(2, -1);
    }
    void prealloc(size_t count) {
        mask++;
        while (mask <= count) mask <<= 1;
        ht.resize(mask, -1);
        mask--;
    }
    bool count(int v) {
        return ht[hash_1(v)] == v || ht[hash_2(v)] == v;
    }
    void insert(int v, vector<int>& table) {
        if (table[hash_1(v)] == -1) {
            table[hash_1(v)] = v;
            return;
        }
        if (table[hash_2(v)] == -1) {
            table[hash_2(v)] = v;
            return;
        }
        bool use_hash_1 = true;
        for (unsigned i=0; i<mask; i++) {
            int cuckooed;
            if (use_hash_1) {
                cuckooed = table[hash_1(v)];
                table[hash_1(v)] = v;
                use_hash_1 = hash_1(v) == hash_2(cuckooed);
            } else {
                cuckooed = table[hash_2(v)];
                table[hash_2(v)] = v;
                use_hash_1 = hash_2(v) == hash_2(cuckooed);
            }
            v = cuckooed;
            if (v == -1) return;
        }
        rehash(table);
        insert(v, table);
    }
    void rehash(vector<int>& table) {
        mask = (mask<<1) | mask;
        vector<int> newt;
        newt.resize(mask+1, -1);
        for (auto t: table)
            if (t != -1)
                insert(t, newt);
        swap(table, newt);
    }
    void insert(int v) {
        if (count(v)) return;
        insert(v, ht);
    }
};
