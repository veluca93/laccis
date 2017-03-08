#include <cstdio>
#include <unordered_set>
#include <vector>
#include "cuckoo.h"

using namespace std;

int N1, N2, M1, M2;
cuckoo_hash_set* g1;
vector<int>* g1_iter;
int *c1;
cuckoo_hash_set* g2;
vector<int>* g2_iter;
int *c2;

class iso_t {
    private:
        vector<int> present;
        int* direct;
        int* inverse;
    public:
        iso_t(size_t g1_size, size_t g2_size) {
            direct = new int[g1_size];
            inverse = new int[g2_size];
            for (size_t i=0; i<g1_size; i++)
                direct[i] = -1;
            for (size_t i=0; i<g2_size; i++)
                inverse[i] = -1;
        }
        ~iso_t() {
            delete[] direct;
            delete[] inverse;
        }
        void append(int a, int b) {
            present.push_back(a);
            direct[a] = b;
            inverse[b] = a;
        }
        void print() {
            printf("{");
            for (auto x: present) {
                printf("%d, ", x);
            }
            printf("\b\b} -> {");
            for (auto x: present) {
                printf("%d, ", direct[x]);
            }
            printf("\b\b}\n");
            fflush(stdout);
        }
        void pop() {
            int to_erase = present.back();
            int other = direct[to_erase];
            direct[to_erase] = -1;
            inverse[other] = -1;
            present.pop_back();
        }
        bool direct_has(int d) {
            return direct[d] != -1;
        }
        bool inverse_has(int i) {
            return inverse[i] != -1;
        }
        int get_direct(int d) {
            return direct[d];
        }
        int get_inverse(int i) {
            return inverse[i];
        }
};

int rts = 0;

void bk(iso_t& iso,
        unordered_set<int>& neigh,
        vector<bool>& excl) {
    rts++;
    bool maximal = true;
    vector<int> friends;
    vector<int> candidates;
    vector<int> excl_added;
    vector<int> diff;
    vector<int> Neigh(neigh.begin(), neigh.end());
    for (auto n: Neigh) {
        if (!maximal && excl[n]) continue;
        friends.clear();
        candidates.clear();
        for (auto d: g1_iter[n]) {
            if (iso.direct_has(d))
                friends.push_back(iso.get_direct(d));
        }
        for (auto c: g2_iter[*friends.begin()]) {
            if (c1[n] != c2[c]) continue;
            bool ok = true;
            for (auto f: friends) 
                if (!g2[f].count(c)) {
                    ok = false;
                    break;
                }
            if (iso.inverse_has(c)) ok = false;
            if (ok) candidates.push_back(c);
        }
        for (auto c: candidates) {
            bool ok = true;
            for (auto v: g2_iter[c])
                if (iso.inverse_has(v) && !g1[n].count(iso.get_inverse(v))) {
                    ok = false;
                    break;
                }
            if (!ok) continue;
            maximal = false;
            if (excl[n]) break;
            diff.clear();
            iso.append(n, c);
            for (auto v: g1_iter[n])
                if (!neigh.count(v) && !iso.direct_has(v))
                    diff.push_back(v);
            for (auto v: diff)
                neigh.insert(v);
            neigh.erase(n);
            bk(iso, neigh, excl);
            neigh.insert(n);
            for (auto v: diff)
                neigh.erase(v);
            iso.pop();
        }
        if (!excl[n]) {
            excl_added.push_back(n);
            excl[n] = 1;
        }
    }
    if (maximal)
        iso.print();
    for (auto e: excl_added)
        excl[e] = 0;
}

void bk_start() {
    vector<bool> excl(N1);
    iso_t iso(N1, N2);
    for (int i=0; i<N1; i++) {
        unordered_set<int> neigh(g1_iter[i].begin(), g1_iter[i].end());
        for (int j=0; j<N2; j++) {
            if (c1[i] != c2[j]) continue;
            iso.append(i, j);
            bk(iso, neigh, excl);
            iso.pop();
            excl[i] = 1;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s g1 g2\n", argv[0]);
        return 1;
    }
    FILE* f1 = fopen(argv[1], "r");
    fscanf(f1, "%d%d", &N1, &M1);
    g1 = new cuckoo_hash_set[N1];
    g1_iter = new vector<int>[N1];
    c1 = new int[N1];
    for (int i=0; i<N1; i++) fscanf(f1, "%d", c1+i);
    for (int i=0; i<M1; i++) {
        int a, b;
        fscanf(f1, "%d%d", &a, &b);
        g1[a].insert(b);
        g1[b].insert(a);
        g1_iter[a].push_back(b);
        g1_iter[b].push_back(a);
    }
    FILE* f2 = fopen(argv[2], "r");
    fscanf(f2, "%d%d", &N2, &M2);
    g2 = new cuckoo_hash_set[N2];
    g2_iter = new vector<int>[N2];
    c2 = new int[N2];
    for (int i=0; i<N2; i++) fscanf(f2, "%d", c2+i);
    for (int i=0; i<M2; i++) {
        int a, b;
        fscanf(f2, "%d%d", &a, &b);
        g2[a].insert(b);
        g2[b].insert(a);
        g2_iter[a].push_back(b);
        g2_iter[b].push_back(a);
    }
    bk_start();
}
